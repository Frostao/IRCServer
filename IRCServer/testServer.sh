#!/usr/bin/expect

spawn telnet localhost 1234
expect "'^]'."
send "ADD-USER superman clarkkent\r\n"
expect "OK\r\n"
spawn telnet localhost 1234
expect "'^]'."
send "ADD-USER spiderman peterpark\r\n"
expect "OK\r\n"
spawn telnet localhost 1234
expect "'^]'."
send "ADD-USER aquaman xyz\r\n"
expect "OK\r\n"
spawn telnet localhost 1234
expect "'^]'."
send "ADD-USER mary poppins\r\n"
expect "OK\r\n"
spawn telnet localhost 1234
expect "'^]'."
send "GET-ALL-USERS superman clarkkent\r\n"
expect "OK\r\n"
