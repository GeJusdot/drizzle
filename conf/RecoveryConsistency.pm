package GenTest::Reporter::RecoveryConsistency;

require Exporter;
@ISA = qw(GenTest::Reporter);

use strict;
use DBI;
use GenTest;
use GenTest::Constants;
use GenTest::Reporter;

sub monitor {
	my $reporter = shift;
	my $pid = $reporter->serverInfo('pid');

	if (time() > $reporter->testEnd() - 10) {
		say("Sending SIGKILL to mysqld with pid $pid in order to force a recovery.");
		kill(9, $pid);
		return STATUS_SERVER_KILLED;
	} else {
		return STATUS_OK;
	}
}

sub report {
	my $reporter = shift;
	my $binary = $reporter->serverInfo('binary');
	my $language = $reporter->serverVariable('language');
	my $datadir = $reporter->serverVariable('datadir');
	$datadir =~ s{[\\/]$}{}sgio;
	my $recovery_datadir = $datadir.'_recovery';
	my $socket = $reporter->serverVariable('socket');
	my $port = $reporter->serverVariable('port');
	my $pid = $reporter->serverInfo('pid');
	
	say("Sending SIGKILL to mysqld with pid $pid in order to force a recovery.");
	kill(9, $pid);
	sleep(10);

	system("cp -r $datadir $recovery_datadir");
	
	say("Attempting database recovery...");

	my @mysqld_options = (
		'--no-defaults',
		'--core-file',
		'--loose-console',
		'--loose-falcon-debug-mask=65535',
		'--language='.$language,
		'--datadir="'.$recovery_datadir.'"',
		'--socket="'.$socket.'"',
		'--port='.$port
	);

	my $mysqld_command = $binary.' '.join(' ', @mysqld_options).' 2>&1';
	say("Executing $mysqld_command .");

	open(MYSQLD, "$mysqld_command|");
	my $recovery_status = STATUS_OK;
	while (<MYSQLD>) {
		$_ =~ s{[\r\n]}{}siog;
		say($_);
		if ($_ =~ m{exception}sio) {
			$recovery_status = STATUS_DATABASE_CORRUPTION;
		} elsif ($_ =~ m{ready for connections}sio) {
			say("Server Recovery was apparently successfull.") if $recovery_status == STATUS_OK ;
			last;
		} elsif ($_ =~ m{got signal}sio) {
			$recovery_status = STATUS_DATABASE_CORRUPTION;

		}
	}

	say("Checking database consistency...");
	my $dbh = DBI->connect($reporter->dsn());

	my $tables = $dbh->selectcol_arrayref("SHOW TABLES");

        foreach my $table (@$tables) {
                my $average = $dbh->selectrow_array("
                        SELECT (SUM(`int_key`)  + SUM(`int`)) / COUNT(*)
                        FROM `$table`
                ");

                if ($average ne '200.0000') {
                        say("Bad average on table: $table; average: $average");
			$recovery_status = STATUS_DATABASE_CORRUPTION;
                } else {
                        say("Average is $average");
		}
        }

	say("Shutting down the recovered server...");

	if (not defined $dbh) {
		$recovery_status = STATUS_DATABASE_CORRUPTION;
	} else {
		$dbh->func('shutdown', 'admin');
	}

	close(MYSQLD);

	if ($recovery_status > STATUS_OK) {
		say("Recovery has failed.");
	}

	return $recovery_status;
}	

sub type {
	return REPORTER_TYPE_CRASH | REPORTER_TYPE_DEADLOCK | REPORTER_TYPE_SUCCESS | REPORTER_TYPE_PERIODIC | REPORTER_TYPE_SERVER_KILLED;
}

1;
