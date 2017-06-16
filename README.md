# MPI Fleet Problem

The problem statement :
```
Your naval fleet patrols an area comprising 1390 distinct locations. Each vessel can occupy any one of these locations randomly2 at the end of sampling interval3. For a vessel to be able to launch a strike, other vessels must accompany it. The rules for launching a strike are summarised as follows:
1. At least two odd or even numbered vessels must share the same location, at a given point in time, for a strike to be counted. MPI rank 1 to n may be used to number each vessel in the fleet
2. The fleet may generate more than one strike at an instant of time (it will however depend on the number of locations meeting the strike criterion, item 1 above, at that instant of time).
3. There is no limit on the number of vessels in the fleet. The objective is to achieve the highest possible strike rate. It may however be pointed out that increases in fleet size will increase the probability of satisfying Rule no. 1 (above), but doing so will also slow the program owing to higher inter-process communication overheads.
Assume that a set of MPI processes represents the fleet and each MPI process represents a naval vessel.
```

Code Resides in :
```
Source/src/main.c
```

Dependencies :
```
gcc or clang
make 
A working MPI implementation (mpicc, mpirun)
```

Build :
```
cd Source
make
```

Run :
Run this program multiple times using different number of processes. You will notice a difference in strike rate.
```
mpirun build/mpiFleet -np < # no of processes here >
```

# Background

This was one of the tasks in the assignment for the subject 'Advanced Distributed and Parallel Systems' (FIT5139) during my Master of Information Technology course at Monash University, Melbourne. 

# Note

This code was written purely for demonstration and educational purposes. This code has no applicability in real world (production). The license allows you to do whatever you want to do with this code. This code comes with zero guarantee/warranty/Support/Hope.
