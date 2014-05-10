Boids Simulation in Fish
==========================================
Background
----------
Craig Reynolds first simulated flocking behaviour in 1987 with his program Boids. The word boid is short for "bird-like object" or "bird-oid". Each individual actor in a flock responds only to its local surroundings, resulting in a visually appealing animation reminiscent of a flock of birds, school of fish or swarm of insects.

Boids also gives us an insight into how software experiments can be used as a tool in understanding biological phenomena. If a school of fish can be modelled in such a way that can also include data-based parameters such as ocean currents, temperatures, predators and food availability, scientists could potentially predict migratory patterns of fish.  

Implementation
--------------
I propose for my project an OpenGL implementation that models the behaviors of a school of fish.  The simulation will be written using Python and PyOpenGL. 2D and 3D views should be implemented, with incremental behavioural features added.  Each individual fish will follow the three boid rules:
* Rule 1: Each fish will move towards the centre of mass of neighbouring fish
* Rule 2: Fish will try and keep a small distance away from other objects (including other fish).
* Rule 3: Fish will try to match velocity with nearby fish.

Behaviours can then be added to the fish, such as predator/prey behavior.

Initially, a 2D view will be developed.  To add interactivity, the mouse cursor will be used in 2 ways.  When a user moves the mouse over the screen, it will act as a 'predator' in the scene, causing the fish to move away from the cursor.  When clicked, the cursor will turn into 'prey'.  This should cause some interesting animation and add a lot of interactivity to the visualisation.  Ideally the movement should be fluid and reminiscent of fish schooling behaviour.

Afterwards, a basic 3D model is proposed - the fish will follow the same rules but within an extra dimension.

This project aims to contain the following visualisations:
* 2D & 3D Rendering of fish (Simple shapes)
* Camera Control (for 3D view)
* Basic animation of fish movement
* User interactivity (Mouse movement and click)

This project will also allow for me to explore texturing and more advanced modelling and animation for the fish,
should time permit.
