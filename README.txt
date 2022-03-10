Evan Sinasac - 1081418
INFO 6028 Graphics Final

Controls:
WASD	- Move the camera around the scene
QE	- Move the camera up/down
B	- Turn debug triangles(normals) on
M	- Switch to Wireframe/back to Solid
ESC	- Close the program

The camera starts outside the dungeon, so I would move up and forward to see the insides.

I did everything up to question 7, so I did not do the time of day changing with the spotlights, nor the ages passing with moss growing.
The crystals are a little bit of a mess, sorry about that, but I've also shown my ability to sort the transparent objects in previous projects, just wasn't sure what to do about the emissive texture stuff and didn't get back to it before submission.
The Sun Shafts for question 6 still need a bit of work, but they're there, and they're using discard transparency.
There was no mention of the dungeon having a ceiling, even though logically it makes sense, so the reason why there is no ceiling is for demo purposes of being able to see into the dungeon from above :)

The program is quite laggy due to the number of models being drawn at all times.  I made this a little better using the sugestions in your email, checking if a light is too far away from a pixel to matter and drawing meshes a certain distance from the camera in wireframe.  I found that despite this it is still very laggy, so if you want to be able to move around faster, uncomment the "else if" on line 593 in the main.cpp, and comment out lines 602-647.  This makes the program only draw meshes within 100 units of the camera, rather than wireframe.  In this case, pressing L will switch everything to wireframe.
Actually, as I was getting ready to zip up the project, I realized I never tested Release mode, so I did just to make sure it works, and it actually runs pretty smoothly.  So, rather than editing the code (unless you want to), running it in Release mode should be fine :)
