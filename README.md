# install
run `make`

# usage
```
usage: pmread PID [OPTIONS] REGIONS...
read map REGIONS of PID and writes to stdout
REGIONS
	<addr start>-<addr end>
	all                        read all REGIONS
	inode:<inodeid>            read REGION by inodeid
	path:<path>                read REGION by path
	dev:<majorid>:<minorid>    read REGION by dev
	major:<majorid>            read REGION by majorid
	minor:<minorid>            read REGION by minorid
OPTIONS
	-h, --help                 shows usage and exit
	list                       list all REGIONS and exit
examples
	pmread PID 7ff14e581000-7ff14e584000
	pmread PID all
	pmread PID path:[heap] path:[stack] path:/path/to/file
	pmread PID inode:0
```