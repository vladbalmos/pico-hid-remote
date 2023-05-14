fn button:
    - long press - start pairing
    - double press - show baterry level
    
toggle play btn
    - wakeup from sleep if sleeping
    - long press - connect to prev host if paired
    
on startup:
    - not discoverable
    - not connectable
    - create timer to check for pair, disable everything in 30 sec
    
on paired:
    - not discoverable
    - connectable
    - avrcp_connect()
    
on lost connection:
    - if previously paired:
        - retry connection for 60 sec, then goto sleep if failed
    
