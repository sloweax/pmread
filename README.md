# install
run `make`

# usage
```
usage: pmread PID [OPTIONS] REGIONS...
read map REGIONS of PID and writes to stdout
REGIONS
	<addr start>-<addr end>
	all                        (read all REGIONS)
	inode:<inodeid>            (read REGION by inodeid)
	path:<path>                (read REGION by path)
OPTIONS
	list                       (list all REGIONS and exit)
examples
	pmread PID 7ff14e581000-7ff14e584000
	pmread PID all
	pmread PID path:[heap] path:[stack] path:/path/to/file
	pmread PID inode:0
```
