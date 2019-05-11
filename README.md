# Training NEAT Neural Network With Ray Casting in Unreal Engine 4
![](https://raw.githubusercontent.com/CynicalApe/NEATSelfDrivingCarUE4/master/Additional/sensor.png)
## How It Works:
Each car has 2 sensor components in front that sends a ray. If the sensors hit anything they return the distance between the sensor and the object. 
We feed this information + car's own velocity vector and acceleration vector to our neural network. Neural network is created by a method called NeuroEvolution of Augmenting Topologies (NEAT)

![](https://raw.githubusercontent.com/CynicalApe/NEATSelfDrivingCarUE4/master/Additional/ray_cast_gif.gif)

We also have some checkpoints on the map, these are just used for giving additional points to the cars that manage to follow the path so that the population can converge faster. 

## Current State:

Managed to increase population size to 750 cars. Each car has 2 sensors and each sensor fires 2 rays every frame so that's 3000 rays per frame + other stuff at 30FPS. 

![](https://raw.githubusercontent.com/CynicalApe/NEATSelfDrivingCarUE4/master/Additional/sample.gif)


## Demo Video Here:

[![](https://img.youtube.com/vi/Wk5aBoxYTAM/0.jpg)](https://youtu.be/Wk5aBoxYTAM)


## References:
[Evolving Neural Networks through Augmenting Topologies](http://nn.cs.utexas.edu/downloads/papers/stanley.ec02.pdf)
