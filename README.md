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
   
2. `tail -f` the log file and pass to `hitter` with pipe

   usage:
   
   `tail -f LOGFILE | ./hitter MAIN_KEY SUB_KEY`

   for example:
   
   `tail -f /tmp/path.log | ./hitter imp clk`
   
   and it will print out informatioin like these:
   
   ```
   mainkey (imp):
      100
   subkey (clk):
      30
   no-hit subkey (clk):
      0
   hit rate:
      30.00%
   ```
  
  # how to compile
  
  just make it.
  
  `make`
  
  # copyright
  
  FreakOut inc.  
  itsusony@fout.jp
