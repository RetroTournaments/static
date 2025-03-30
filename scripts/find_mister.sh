# This command finds all ssh able devices (like misters)
nmap -T 4 --min-rate=5000 -p 22 --open -sV 192.168.1.0/24 
