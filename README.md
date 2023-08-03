### Problem defenition
A set of `N` points is placed in a two dimentional plane. The coordinates of each point `P` are defined as follows:

$x=\frac{x_2 - x_1}{2}  \times sin( t \times \frac {\pi}{2}) + \frac {x_2 + x_1}{2}$<br>
$y = a \times x + b$<br>
where $x_1, x_2, a, b$ are constant (double) parameters predifined for each point `P`.

Given a time value `t`, we want to find if there exist at least ***3*** points that satisfies the `Proximity criteria`.<br>
a point `P` from the set satisfies the criteria if there exist at least `K` points in the set with a distance closer than a given value `D` from the point `P`.<br>
the proximity criteria will be checked for `tCount` + 1 (`tCount` is a given integer) values of `t` that is defined as follows:<br>
$t=\frac {2i}{tCount} - 1, i=0,1,2...tCount$

### Input data and output file
For your convinence, an input file (`input.txt`) is supplied with a set of points.
in order to create your own input file please follow the input guideline:

```
N K D tCount
id x1 x2 a b
id x1 x2 a b
...
id x1 x2 a b

where:
N (int)               number of points in the set.
K (int)               minimal number of points to satisfy the proximity criteria.
D (float)             maximum distance (exclusive) between points to satisfy the criteria.
tCount (int)          number of checks to preform.

id (int)              point id.
x1, x2, a, b (float)  points parameters.
```

the output file (`output.txt`) will contain infromation about the results found for points that satisfy the criteria.<br>
For each `t` where **3** points satisfy the criteria found, a line with the points IDs and the time will be printed: 
```
Points pointIDx, pointIDy, pointIDz satisfy the Proximity Criteria at t = t(n)
```
in case that no points found for any value of `t`, the output will be:
```
There were no 3 points found for any t.
```

## How to use - Linux machines
1. clone the repository and `cd` to its directory.
2. using `make`, compile and build the executable file.
3. run using `make run`.<br> 
the project demonstrates distributed parallel computing so it can also be run on two machines.<br>
in order to do so, add a new file called `ips.txt` in the repository folder with the IP addresses of both machines.<br>
***both machines have to have be on the same netwrok and have the source code in the same path.***<br>
make sure the porgram is built on both machines, then use in **one** of the machinces `make runOn2` to execute the code on both.
5. the program will create the `output.txt` file where you can see the results.
