## op21.engine
* 1) clone repository
* 2) edit mounts/mount.cmd
	* * set s: for cloned source tree
	* * set x: for compilers working files
	* * set y: for result output (maybe stalker root directory)
* 3) exec mount.cmd
* 4) Edit if necessary
	* * for vs2012 GlobalProperties_2012.props
	* * for vs2015 GlobalProperties.props
	and specify your directorys
* 4) open solution 
	* * for VS2012 ```s:\xr_3da\XR_3DA.sln```
	* * for VS2015 ```s:\XR_3DA.sln```
* 5) build (it may take a few builds run, there are several problems with projects dependences)
