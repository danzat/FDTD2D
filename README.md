FDTD2D
======

A 2D simulation of EM wave propagation using an FDTD method

This is the solution I wrote for a mid-term in an EM simulation course I took.
The simulation is an implementation of an FDTD (https://en.wikipedia.org/wiki/Finite-difference_time-domain_method) method for an impulse wave propagating in a parallel-plate waveguide with an obstacle in the middle.
I thought it would be cool to have a video output for the simulation, so I decided writing it with C.
I paint each frame to a Cairo surface, and then use Theora to stitch all the frames (needless to say libcairo and libtheora are dependencies).
