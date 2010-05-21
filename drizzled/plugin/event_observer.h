/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Definitions required for EventObserver plugin
 *
 *  Copyright (C) 2010 PrimeBase Technologies GmbH, Germany
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Barry Leslie
 *
 * 2010-05-12
 */

/*
 * How to add a new event:
 *
 * In event_observer.h
 * 1: Add your event to EventType.
 * 2: Add it to EventObserver::eventName().
 * 3: Cerate a EventData class for it based on SessionEventData, SchemaEventData, 
 *  or TableEventData depending on the event type class.
 * 4: Add a static method to the EventObserver class, similar to beforeInsertRecord() for example,
 *  that will be called by drizzle.
 *
 * In event_observer.cc
 * 5: Impliment your static event method, similar to beforeInsertRecord() for example.
 * 6: Depending on the event type class add an event vector for it to the SessionEventObservers,
 *  SchemaEventObservers, or TableEventObservers class.
 *
 */
#ifndef DRIZZLED_PLUGIN_EVENT_OBSERVER_H
#define DRIZZLED_PLUGIN_EVENT_OBSERVER_H

#include "drizzled/plugin/plugin.h"

#include <string>

namespace drizzled
{

class TableShare;

namespace plugin
{
  class EventObserverList;
  class EventData;


class EventObserver : public Plugin
{
  EventObserver();
  EventObserver(const EventObserver &);
  EventObserver& operator=(const EventObserver &);
public:
  explicit EventObserver(std::string name_arg)
    : Plugin(name_arg, "EventObserver")
  {}
  virtual ~EventObserver() {}

   enum EventType{
      /* Session events: */
      BEFORE_CREATE_DATABASE, AFTER_CREATE_DATABASE, 
      BEFORE_DROP_DATABASE,   AFTER_DROP_DATABASE,
      
      /* Schema events: */
      BEFORE_DROP_TABLE,   AFTER_DROP_TABLE, 
      BEFORE_RENAME_TABLE, AFTER_RENAME_TABLE, 
      
      /* Table events: */
      BEFORE_INSERT_RECORD,   AFTER_INSERT_RECORD, 
      BEFORE_UPDATE_RECORD,   AFTER_UPDATE_RECORD, 
      BEFORE_DELETE_RECORD,   AFTER_DELETE_RECORD,
      
      /* The max event ID marker. */
      MAX_EVENT_COUNT
    };

  static const char *eventName(EventType event) 
  {
    switch(event) 
    {
    case BEFORE_DROP_TABLE:
      return "BEFORE_DROP_TABLE";
    
    case AFTER_DROP_TABLE:
      return "AFTER_DROP_TABLE";
    
    case BEFORE_RENAME_TABLE:
      return "BEFORE_RENAME_TABLE";
      
    case AFTER_RENAME_TABLE:
      return "AFTER_RENAME_TABLE";
      
    case BEFORE_INSERT_RECORD:
       return "BEFORE_INSERT_RECORD";
      
    case AFTER_INSERT_RECORD:
       return "AFTER_INSERT_RECORD";
      
    case BEFORE_UPDATE_RECORD:
      return "BEFORE_UPDATE_RECORD";
               
    case AFTER_UPDATE_RECORD:
      return "AFTER_UPDATE_RECORD";
      
    case BEFORE_DELETE_RECORD:
      return "BEFORE_DELETE_RECORD";

    case AFTER_DELETE_RECORD:
      return "AFTER_DELETE_RECORD";

    case BEFORE_CREATE_DATABASE:
      return "BEFORE_CREATE_DATABASE";

    case AFTER_CREATE_DATABASE:
      return "AFTER_CREATE_DATABASE";

    case BEFORE_DROP_DATABASE:
      return "BEFORE_DROP_DATABASE";

    case AFTER_DROP_DATABASE:
      return "AFTER_DROP_DATABASE";
      
    case MAX_EVENT_COUNT:
      break;
   }
    
    return "Unknown";
  }
  
  /*==========================================================*/
  /* registerEvents() must be implemented to allow the plugin to
   * register which events it is interested in.
   */
  virtual void registerTableEventsDo(TableShare &, EventObserverList &){}
  virtual void registerSchemaEventsDo(const std::string &/*db*/, EventObserverList &) {}
  virtual void registerSessionEventsDo(Session &, EventObserverList &) {}

  virtual bool observeEventDo(EventData &)= 0;

  /*==========================================================*/
  /* Static access methods called by drizzle: */
  static bool addPlugin(EventObserver *handler);
  static void removePlugin(EventObserver *handler);
  
  /*==========================================================*/
  /* Register an event of interest for this plugin. 
   * This is called from with in the plugin when registering itself.
   *
   * The position field is used to indicate the order the event observer is to be 
   * called. If the event observer must be called before any other observer then 
   * the position must be set to 1. If it must be called last then the position must be 
   * set to -1. A position of 0 indicated the position doesn't matter.
   *
   * If 2 plugins require the same position then which is called first in not guarenteed.
   * In this case a warrning will be logged but execution will continue.
   * 
   * It is good practice that if the event position matters not to hard code the position
   * but supply a systen variable so that it can be set at runtime so that the user can
   * decide which event should be called first.
   */
  void registerEvent(EventObserverList &observers, EventType event, int32_t position= 0); 

  /*==========================================================*/
  /* Called from drizzle to register all events for all event plugins 
   * interested in this table. 
   */
  static void registerTableEvents(TableShare &table_share); 
  static void deregisterTableEvents(TableShare &table_share); 

  /*==========================================================*/
  /* Called from drizzle to register all events for all event plugins 
  * interested in this database. 
  */
  static void registerSchemaEvents(Session &session, const std::string &db); 
  static void deregisterSchemaEvents(Session &session, const std::string &db); 

  /*==========================================================*/
  /* Called from drizzle to register all events for all event plugins 
  * interested in this session. 
  */
  static void registerSessionEvents(Session &session); 
  static void deregisterSessionEvents(Session &session); 

 
  /*==========================================================*/
  /* Static meathods called by drizzle to notify interested plugins 
   * of a schema an event,
   */
  static bool beforeDropTable(Session &session, TableIdentifier &table);
  static bool afterDropTable(Session &session, TableIdentifier &table, int err);
  static bool beforeRenameTable(Session &session, TableIdentifier &from, TableIdentifier &to);
  static bool afterRenameTable(Session &session, TableIdentifier &from, TableIdentifier &to, int err);

  /*==========================================================*/
  /* Static meathods called by drizzle to notify interested plugins 
   * of a table an event,
   */
  static bool beforeInsertRecord(Session &session, TableShare &table_share, unsigned char *buf);
  static bool afterInsertRecord(Session &session, TableShare &table_share, const unsigned char *buf, int err);
  static bool beforeDeleteRecord(Session &session, TableShare &table_share, const unsigned char *buf);
  static bool afterDeleteRecord(Session &session, TableShare &table_share, const unsigned char *buf, int err);
  static bool beforeUpdateRecord(Session &session, TableShare &table_share, const unsigned char *old_data, unsigned char *new_data);
  static bool afterUpdateRecord(Session &session, TableShare &table_share, const unsigned char *old_data, unsigned char *new_data, int err);

  /*==========================================================*/
  /* Static meathods called by drizzle to notify interested plugins 
   * of a table an event,
   */
  static bool beforeCreateDatabase(Session &session, const std::string &db);
  static bool afterCreateDatabase(Session &session, const std::string &db, int err);
  static bool beforeDropDatabase(Session &session, const std::string &db);
  static bool afterDropDatabase(Session &session, const std::string &db, int err);

};


/* EventObserver data classes: */
//======================================
class EventData
{
public:
  EventObserver::EventType event;
   
  EventData(EventObserver::EventType event_arg): 
    event(event_arg),
    observerList(NULL)
    {}
  virtual ~EventData(){}
  
  // Call all the event observers that are registered for this event.
  virtual bool callEventObservers();
  
protected:
  EventObserverList *observerList;
  
};

//-----
class SessionEventData: public EventData
{
public:
  Session &session;
  
  SessionEventData(EventObserver::EventType event_arg, Session &session_arg): 
    EventData(event_arg),
    session(session_arg)
    {}
  virtual ~SessionEventData(){}
  
  
  // Call all the event observers that are registered for this event.
  virtual bool callEventObservers();
};

//-----
class SchemaEventData: public EventData
{
public:
  Session &session;
  const std::string &db;
  
  SchemaEventData(EventObserver::EventType event_arg, Session &session_arg, const std::string &db_arg): 
    EventData(event_arg),
    session(session_arg),
    db(db_arg)
    {}
  virtual ~SchemaEventData(){}
  
  
  // Call all the event observers that are registered for this event.
  virtual bool callEventObservers();
};

//-----
class TableEventData: public EventData
{
public:
  Session &session;
  TableShare &table;  
  
  TableEventData(EventObserver::EventType event_arg, Session &session_arg, TableShare &table_arg): 
    EventData(event_arg),
    session(session_arg),
    table(table_arg)
    {}
  virtual ~TableEventData(){}
  
  
  // Call all the event observers that are registered for this event.
  virtual bool callEventObservers();
};

//-----
class BeforeCreateDatabaseEventData: public SessionEventData
{
public:
  const std::string &db;

  BeforeCreateDatabaseEventData(Session &session_arg, const std::string &db_arg): 
  SessionEventData(EventObserver::BEFORE_CREATE_DATABASE, session_arg), 
  db(db_arg)
  {}  
};

//-----
class AfterCreateDatabaseEventData: public SessionEventData
{
public:
  const std::string &db;
  int err;

  AfterCreateDatabaseEventData(Session &session_arg, const std::string &db_arg, int err_arg): 
  SessionEventData(EventObserver::AFTER_CREATE_DATABASE, session_arg), 
  db(db_arg), 
  err(err_arg)
  {}  
};

//-----
class BeforeDropDatabaseEventData: public SessionEventData
{
public:
  const std::string &db;

  BeforeDropDatabaseEventData(Session &session_arg, const std::string &db_arg): 
  SessionEventData(EventObserver::BEFORE_DROP_DATABASE, session_arg), 
  db(db_arg)
  {}  
};

//-----
class AfterDropDatabaseEventData: public SessionEventData
{
public:
  const std::string &db;
  int err;

  AfterDropDatabaseEventData(Session &session_arg, const std::string &db_arg, int err_arg): 
  SessionEventData(EventObserver::AFTER_DROP_DATABASE, session_arg), 
  db(db_arg), 
  err(err_arg) 
  {}  
};

//-----
class BeforeDropTableEventData: public SchemaEventData
{
public:
  TableIdentifier &table;

  BeforeDropTableEventData(Session &session_arg, TableIdentifier &table_arg): 
  SchemaEventData(EventObserver::BEFORE_DROP_TABLE, session_arg, table_arg.getSchemaName()), 
  table(table_arg)
  {}  
};

//-----
class AfterDropTableEventData: public SchemaEventData
{
public:
  TableIdentifier &table;
  int err;

  AfterDropTableEventData(Session &session_arg, TableIdentifier &table_arg, int err_arg): 
  SchemaEventData(EventObserver::AFTER_DROP_TABLE, session_arg, table_arg.getSchemaName()), 
  table(table_arg), 
  err(err_arg)
  {}  
};

//-----
class BeforeRenameTableEventData: public SchemaEventData
{
public:
  TableIdentifier &from;
  TableIdentifier &to;

  BeforeRenameTableEventData(Session &session_arg, TableIdentifier &from_arg, TableIdentifier &to_arg): 
  SchemaEventData(EventObserver::BEFORE_RENAME_TABLE, session_arg, from_arg.getSchemaName()), 
  from(from_arg), 
  to(to_arg)
  {}  
};

//-----
class AfterRenameTableEventData: public SchemaEventData
{
public:
  TableIdentifier &from;
  TableIdentifier &to;
  int err;

  AfterRenameTableEventData(Session &session_arg, TableIdentifier &from_arg, TableIdentifier &to_arg, int err_arg): 
  SchemaEventData(EventObserver::AFTER_RENAME_TABLE, session_arg, from_arg.getSchemaName()), 
  from(from_arg), 
  to(to_arg), 
  err(err_arg)
  {}  
};

//-----
class BeforeInsertRecordEventData: public TableEventData
{
public:
  unsigned char *row;

  BeforeInsertRecordEventData(Session &session_arg, TableShare &table_arg, unsigned char *row_arg): 
  TableEventData(EventObserver::BEFORE_INSERT_RECORD, session_arg, table_arg), 
  row(row_arg)
  {}  
};

//-----
class AfterInsertRecordEventData: public TableEventData
{
public:
  const unsigned char *row;
  int err;

  AfterInsertRecordEventData(Session &session_arg, TableShare &table_arg, const unsigned char *row_arg, int err_arg): 
  TableEventData(EventObserver::AFTER_INSERT_RECORD, session_arg, table_arg), 
  row(row_arg),
  err(err_arg)
  {}  
};

//-----
class BeforeDeleteRecordEventData: public TableEventData
{
public:
  const unsigned char *row;

  BeforeDeleteRecordEventData(Session &session_arg, TableShare &table_arg, const unsigned char *row_arg): 
  TableEventData(EventObserver::BEFORE_DELETE_RECORD, session_arg, table_arg), 
  row(row_arg)
  {}  
};

//-----
class AfterDeleteRecordEventData: public TableEventData
{
public:
  const unsigned char *row;
  int err;

  AfterDeleteRecordEventData(Session &session_arg, TableShare &table_arg, const unsigned char *row_arg, int err_arg): 
  TableEventData(EventObserver::AFTER_DELETE_RECORD, session_arg, table_arg), 
  row(row_arg),
  err(err_arg)
  {}  
};

//-----
class BeforeUpdateRecordEventData: public TableEventData
{
public:
  const unsigned char *old_row;
  unsigned char *new_row;

  BeforeUpdateRecordEventData(Session &session_arg, TableShare &table_arg,  
    const unsigned char *old_row_arg, 
    unsigned char *new_row_arg): 
      TableEventData(EventObserver::BEFORE_UPDATE_RECORD, session_arg, table_arg), 
      old_row(old_row_arg),
      new_row(new_row_arg)      
      {}  
};

//-----
class AfterUpdateRecordEventData: public TableEventData
{
public:
  const unsigned char *old_row;
  const unsigned char *new_row;
  int err;

  AfterUpdateRecordEventData(Session &session_arg, TableShare &table_arg, 
    const unsigned char *old_row_arg, 
    const unsigned char *new_row_arg, 
    int err_arg): 
      TableEventData(EventObserver::AFTER_UPDATE_RECORD, session_arg, table_arg), 
      old_row(old_row_arg),
      new_row(new_row_arg),
      err(err_arg)
      {}  
};

//=======================================================

} /* namespace plugin */
} /* namespace drizzled */

#endif /* DRIZZLED_PLUGIN_EVENT_OBSERVER_H */