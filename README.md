# Moho Export to Lightning

In this repo, it shows how to use Lua script export animation from Moho, and write relevant information 
such as bone rotation, switch layer, smart bone, camera configuration, document size etc to local files. 
Lightning will parse the files and prepare correspondent angles, facial element, scale, position in panel text to generate images frame by frame.


## Moho Export
1. Install Moho Pro 12

2. To export character animation from Moho, you can use the lua script wangyiMohoExporter.lua.
   In wangyiMohoExporter.lua, 
   change wangyiMohoExporter.toPath = "/Users/Yiyang.Wang/documents/moho/exported/" .. moho.document:Name() .. ".txt" to
   the path where you want the exported files be.  

3. Copy wangyiMohoExporter.lua under MOHO_INSTALL_PATH\scripts\menu\tools. Open Moho, you should see in menu: Scripts->Tools->wangyiMohoExporter

4. Open the moho project "BitMoho-scene test.moho", click menu Scripts->Tools->wangyiMohoExporter. Wait a couple of seconds, but it can be faster.
Under wangyiMohoExporter.toPath, there should be the txt file containing all the animation information in Moho.



## Lightning
1. Update svgBuilder under Lightning to svgBuilder in this repo, in the main function:

        wangyiGenAnimation(
        "/Users/Yiyang.Wang/documents/moho/exported/BitMoho-scene test.moho.txt",//input moho text file path
        "/exported/svgs/");//output image sequence files path
        return 0;
2. Change "/Users/Yiyang.Wang/documents/moho/exported/BitMoho-scene test.moho.txt" path to be consistent with wangyiMohoExporter.toPath.
 
3. Run the svgBuilder, you will see svg images generated, each per frame. An animation file called "exported.webp" will also be generated and visualized in Chrome browser. The output is by default at ".../snapchat/Dev/monorepo/rendering/lightning/exported/svgs/". If you are running for the first time, make sure to create "exported" and "svgs" folders first.

## Automation
Code not commited to this repo yet. 
1. In Moho Lua script, it can call Lightning library. This process succeeded on local machine, but code not committed here yet.
Check https://github.sc-corp.net/ywang/CLua/tree/master/cpplib for technical details
