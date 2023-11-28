# Multi-ParticleSimulator
One Particle simulator with different solvers for different results
This program uses SFML.

# Move around
You can move around the camera by clicking and draging rith-mouse.
You can zoom in and out with the scrollwheel.

# Choose the simulator
There is a default parent class ParticleSimulator that is used as a base for everything necessary in the simulation. The other simulators are implementation of this base class and show different ways I wanted the particles to interact.
You can choose which kind of simulator you want to use in the main by uncommenting its initialization and commenting the ones you don't want.

There are a lot of parameters you can change in each simulation and in the default struct world_t to get a huge variety of behaviors (although some may not be very interesting).
The parameters set in the code right now give, in my opinion, interesting or aesthetically pleasing results.

# Save the simulation
There is the possibility to save the simulation in the binary file "saves/m_particles.bin" and play it afterwards.
This allows to review simulations as many times as you want and even play them faster. This means simulations with a lot of particles (so with low framerate) can be replayed at a not-boring pace.

# Thank you to johnBuffer
For inspiring me in finally doing this project.
Go check [his Simulator](https://github.com/johnBuffer/VerletSFML-Multithread), it is far better than what I could do.
