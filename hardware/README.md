# Hardware Notes.

Constructing the BuzzKill hardware can be divided into two distinct steps: fabricating the PCB, and assembling (populating) the PCB.

## Fabricating the PCB

A [schematic](../hardware/BuzzKill_schematic.png) is provided if you want to start with a breadboard prototype or create your own PCB layout. If you do lay out your own PCB, you can also use the PCB images in the [hardware](../hardware/) folder as a reference or starting point.

If you want to utilize the standard PCB as-is, the [necessary files](../hardware/BuzzKill_gerber_files.zip) (Gerbers and drill files) are provided in .zip format. Most fabrication houses will accept the .zip file directly as an upload, and automatically pull out the needed files. With some you may need to unzip the file and separately submit invidiual files such as *.GTL for the top layer, *.GBL for the bottom layer, *.DRL for drill files, etc. Each fabricator has its own process in this regard.

You will usually have the option to have a stencil made at the same time. While this can make assembling the PCB a bit easier, especially if it is your first time, it's generally not neccessary for a board like this.

## Assembling the PCB

Assembling refers to soldering all the components in place on the PCB. The simplest way to do this is through a reflow process: applying solder paste to the PCB, carefully placing the parts on the pasted areas, and heating the PCB until the solder paste melts.

I won't go into detail here, as there are plenty of available resources covering every aspect of do-it-yourself reflow. I will only note that this particular board is quite easy to reflow with no special equipment (a basic hotplate or stovetop setup works fine), due to its small size and simple components. I have also not found a stencil to be necessary, due to the fairly wide separation of most components. The one exception is the amplifier IC pins, which are pretty closely spaced. The key is to apply the solder paste sparingly, using only enough to cover each pad to about 1mm thickness. It doesn't have to be super-neat either, some people actually lay it down in a single line across all the IC pads and then smooth it out a bit. As long as there is not too much paste, it will naturally pull itself away from the gaps and onto the pads as it heats up.

The component locations and designations are not marked on the PCB silkscreen, but the layout makes it simple to see where each part needs to go. Refer to [BuzzKill_component_layout.png](./BuzzKill_component_layout.png) for placement details. Pay attention to the orientation of the ICs, and note that they face in opposite directions! The dots on the silkscreen indicate the general position of pin 1 for each IC. The rest of the components have no specific orientation other than the obvious horizontal or vertical.
