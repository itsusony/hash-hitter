# hash-hitter
a log tool for calcuating the hit of hash

# usage

1. generate logs into a file:

   the string's format in each row need be `TAG:HASHVALUE`
   
   sample:
   
   ```
   imp:ID1000001
   imp:ID1000002
   clk:ID1000001
   clk:ID1000002
   cnv:ID1000001
   cnv:ID1000002
   ...
   ```
   
   ※ you can use anything in `TAG`, such as `register:10001`, and anything in `HASHVALUE`, like `register:MyNameIs007`
   
   you can append data into same file from difference process at same time.
   
   * `perl user_register_log.pl >> /tmp/path.log`
   * `perl user_logined_log.pl >> /tmp/path.log`
   * `cap invoke COMMAND="your commands" ROLES=your_role 2>&1 | unbuffer -p brabra >> /tmp/path.log`
   
   it is good to use `unbuffer -p` before last process, for reduce buffer latency.
   
   `unbuffer -p perl user_register_log.pl >> /tmp/path.log`
   
2. `tail -f` the log file and pass to `hitter` with pipe, and tell which key is a main key.

   usage:
   
   `tail -f LOGFILE | ./hitter MAIN_KEY`

   for example:
   
   `tail -f /tmp/path.log | ./hitter imp`
   
   and it will print out informatioin like these:
   
   ```
   mainkey (imp): 1000000   
                          
   subkey (clk):   33        
   subkey (cnv):   10    
   ```
  
  # how to compile
  
  just make it.
  
  `make`
  
  # highlight
  
  I use a list queue for receiving the data from pipe, to reduce the latency.
  Data will be processed in another thread, and printing out the data at third thread.
  
  All of the data flow is designed on none-blocking.
  
  # copyright
  
  FreakOut inc.  
  itsusony@fout.jp
