#! /usr/bin/python
# -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*-
# vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
#
# Copyright (C) 2010 Patrick Crews
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

""" test_execution:
    code related to the execution of est cases 
    
    We are provided access to a testManager with 
    mode-specific testCases.  We contact the executionManager
    to produce the system and server configurations we need
    to execute a test.

"""

# imports
import os
import sys

class testExecutor():
    """ class for handling the execution of testCase
        objects.  Mode-specific executors inherit
        from me.
 
    """

    def __init__(self, execution_manager, name, verbose, debug):
        self.skip_keys = [ 'execution_manager'
                         , 'system_manager'
                         , 'test_manager'
                         , 'server_manager']
        self.debug = debug
        self.verbose = verbose
        self.initial_run = 1
        self.status = 0 # not running
        self.execution_manager = execution_manager
        self.system_manager = self.execution_manager.system_manager
        self.cmd_prefix = self.system_manager.cmd_prefix
        self.logging = self.system_manager.logging
        self.test_manager = self.execution_manager.test_manager
        self.server_manager = self.execution_manager.server_manager
        self.time_manager = self.system_manager.time_manager
        self.name = name
        self.master_server = None
        self.record_flag=self.execution_manager.record_flag
        self.environment_vars = {}
        self.current_servers = []
        self.current_testcase = None    
        self.current_test_status = None
        self.current_test_retcode = None
        self.current_test_output = None
        self.current_test_exec_time = 0
        self.dirset = { self.name : { 'log': None } }
        self.workdir = self.system_manager.create_dirset( self.system_manager.workdir
                                                        , self.dirset)
        self.logdir = os.path.join(self.workdir,'log')
         
        if self.debug:
            self.logging.debug_class(self)

    def get_testCase(self):
        """ Ask our execution_manager for a testCase to work on """
        
        #self.test_manager.mutex.acquire()
        self.current_testcase = self.test_manager.get_testCase(self.name)
        #self.test_manager.mutex.release()
        if self.debug:
            self.logging.debug("Executor: %s, assigned test: %s" %(self.name
                                            , self.current_testcase.fullname))

    def handle_server_reqs(self):
        """ Get the servers required to execute the testCase """
       
        master_count, slave_count, server_options = self.process_server_reqs()
        (self.current_servers,bad_start) = self.server_manager.request_servers( self.name
                                                              , self.workdir
                                                              , master_count
                                                              , slave_count
                                                              , server_options)
        if self.current_servers == 0 or bad_start:
            # error allocating servers, test is a failure
            return 1
        if self.initial_run:
            self.initial_run = 0
            self.current_servers[0].report()
        self.master_server = self.current_servers[0]
        return 0

    def process_server_reqs(self):
        """ Check out our current_testcase to see what kinds of servers 
            we need up and running.  The executionManager sees to 
            serving the reqs

        """
     
        master_count = self.current_testcase.master_count
        slave_count = self.current_testcase.slave_count
        server_options = self.current_testcase.server_options

        return(master_count, slave_count, server_options)

    def execute(self, start_and_exit):
        """ Execute a test case.  The details are *very* mode specific """
        self.status = 1 # we are running
        keep_running = 1
        if self.verbose:
            self.logging.verbose("Executor: %s beginning test execution..." %(self.name))
        while self.test_manager.has_tests() and keep_running == 1:
            self.get_testCase()
            self.handle_system_reqs()
            bad_start = self.handle_server_reqs()
            if bad_start:
                # Our servers didn't start, we mark it a failure
                self.logging.warning("Problem starting server(s) for test...failing test case")
                self.current_test_status = 'fail'
                self.set_server_status(self.current_test_status)
                output = ''
            if start_and_exit:
                # TODO:  Report out all started servers via server_manager/server objects?
                self.logging.info("User specified --start-and-exit.  dbqp.py exiting and leaving servers running...")
                sys.exit(0)
            else:
                self.execute_testCase()
            self.test_manager.record_test_result( self.current_testcase
                                                , self.current_test_status
                                                , self.current_test_output
                                                , self.current_test_exec_time )
            if self.current_test_status == 'fail' and not self.execution_manager.force:
                self.logging.error("Failed test.  Use --force to execute beyond the first test failure")
                keep_running = 0
        self.status = 0

    def execute_testCase(self):
        """ Do whatever evil voodoo we must do to execute a testCase """
        if self.verbose:
            self.logging.verbose("Executor: %s executing test: %s" %(self.name, self.current_testcase.fullname))

            

    def set_server_status(self, test_status):
        """ We update our servers to indicate if a test passed or failed """
        for server in self.current_servers:
            if test_status == 'fail':
                server.failed_test = 1

   
    def handle_system_reqs(self):
        """ We check our test case and see what we need to do
            system-wise to get ready

        """

        if self.current_testcase.master_sh:
                self.system_manager.execute_cmd("/bin/sh %s" %(self.current_testcase.master_sh))
