# Lost-Boys-Interactive
Summer Internship from May 2021-August 2021

Document giving deeper detail: https://docs.google.com/document/d/1KI3fMLunziFckchhJ9l5K78ikq54usYI4njA7_73p9I/edit?usp=sharing </br>
Tower defense demo video: https://drive.google.com/file/d/1bz3euRhIKMYrrQ6ABfY-W2UzBdaYZYmU/view?usp=sharing </br>
Co-op demo video: https://drive.google.com/file/d/1BizLfsQR_NNaPmF4ho5urwueO8MmLIp0/view?usp=sharing </br>
Procedurally generated maze demo video: https://drive.google.com/file/d/1E5IFFAWxHbvcNXKpYuSkr8DnYZgn5phX/view?usp=sharing </br>

Over the past summer, I became familiar with Unreal Engine 4. As a Junior Programmer at LBI, I began the summer testing and creating a boot camp/ramp up program that experienced engineers/other junior programmers would be able to use to quickly learn UE4. 
This took the form of a Tower Defense game; many parts of the engine (animations, UI, Niagara Effects, landscapes, splines/paths, Blueprints vs. C++, etc.) were required to create the game, and it was my job to figure them out, explain them, and publish a page with details (sadly I cannot post these to the public) for others to read, follow along with, and use as references. 
Once the program was built up to the company's liking, I learned the C++ side of the engine a little more through Tom Looman's tutorials (this led to the creation of the FPS/Stealth game as well as the Co-op game). 
Finally, for the last month or so, I was let loose to explore the engine and create something cool that might lead to new finds for the company. 
What I chose to do was make a procedurally generated map for an "infinite" maze runner game that would be adventure-style in nature. 
As a premise, the player would run through the maze to find different objects (collectible items, pets, and money) and locations (monster fields, villages). 
During their adventure, the maze would serialize itself with instanced static meshes that can easily be created and destroyed (these would be the floors and walls the player would see) so that the game will efficiently run while retaining its maze shape and important location information.
I was able to get the maze fully working (maze shape, loading in/out) and one of the locations (the monster field) fully implemented as well (see project for details).
The inventory system is a work in progress, but I was able to implement picking up objects and storing them for the most part.
