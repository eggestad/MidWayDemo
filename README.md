# Building

gcc -fpic -I/opt/MidWay/include -c server1.c;
gcc -shared -o server1.so server1.o 

gcc -fpic -I/opt/MidWay/include -c kvserver.c
gcc  -shared  -o kvserver.so kvserver.o 

# running

## manuel start for testing

mwserver -l debug `pwd`/server1.so 
mwserver -l debug `pwd`/kvserver.so 

## normal start


# server1.so

## services

### uptime
return what the date command do
### date

### echo
echos back the same data

### event
get the server to issue an event
mwcall event "eventname eventdata"

## stack

### push
mwcall push 1

### peek
mwcall peek 

### pop
mwcall pop

### size
mwcall size

### multi
mwcall multi ddd


# kvserver.so
A simple key/value store
 

## services

### kvput
mwcall -l debug kvput x=xxx
mwcall -l debug kvput x2=xx


### kvget
mwcall -l debug kvget x


### kvdel
mwcall -l debug kvdel x
