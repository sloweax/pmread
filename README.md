#install
run `make`

#usage
```
usage: pmread PID [REGIONS...]
read map REGIONS of PID and writes to stdout
examples:
	pmread PID <addr start>-<addr end>
	pmread PID all (read all REGIONS)
	pmread PID path:[heap] path:[stack] path:/path/to/file (read REGION by path)
	pmread PID inode:0 (read REGION by inodeid)
```
