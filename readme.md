# HomeWork 2:  Building an LSystem Plug-in for Maya.
Anton Khabbaz:  CGGT Master's UPENN

## Goal  

Write a Maya (Autodesk) plugin that interfaces with C++ code and has all the features of a
user friendly maya plugin.  The example here builds an LSystem for maya that
parses grammar and produces shapes that follow L-Systems grammars described in “The Algorithmic Beauty
of Plants" by Przemyslaw Prusinkiewicz and Arstid Lindenmeyer [1].  The main
goal here is to implement the shapes and geometry in Maya, make a good user
interface and follow Maya's requirements by building MObjects that Maya
controls.

##  Base Code and example code Supplied to us:
The base code supplied had and implementation of LSystem.h and LSystem.cpp that
parsed L-system strings and produced grammars that essentially gave a list of
branches where each branch consisted of a pair of 3D points.  The symbols
implement in LSystem.h are:

![Turtle Commands](https://github.com/akhabbaz/MayaPluginHW2/blob/master/GrammarRules.PNG "Table of Turtle Commands")

LSystem.h has a structure called a `Branch` that is a pair of 3D points and that
was used to convert grammars into positions.

The base code also included a cylinder.h class that much like a cube, created
vertices, polygon faces and, and connections that assign for each polygon face
the indices of the vertices that define it.  This follows the OpenGL
conventions.

In addition Maya provide a class animCubeNode which was the bases for
LSystemNode.

## 2.1 Getting Started.

The base code compiled.  The project created a dynamically loaded library or for
Maya an "mll" entitled "LSystemMaya.mll".  That registers a command
`LSystemCmd`. From the script editor typing in LSystemCmd will run the doit
method, printing the output implement me to the script window. Ultimately I set
up my environment to have a plugin directory and a script directory that Maya
checks for all plugins.  This way provide I just have to copy the plugin to the
appropriate directory and Maya will find it when loading it.

##2.2

### a.  
The Doit command was modified to draw and circle, a nurbs line, and to extrude
the circle along the line. 

The code is:
```

	parseArguments(args);
	MGlobal::executeCommand(MString("circle -c ") + circleCenter.x + " " +
		circleCenter.y + " " + circleCenter.z + " " + " -nr " + 
		circleNormal.x + " " + circleNormal.y + " " + circleNormal.z + " " +
		" -r " + radius + "-d 3 -ut 0 -tol 0.01 -s 8 -ch 1 -n nurbsCircle1", true);
	const unsigned  deg = 1;
	const unsigned ncvs = 2;
    	const unsigned spans = ncvs - deg;
	const unsigned nknots = spans + 2 * deg - 1; // number of knots
	MPointArray controlVertices;
	MDoubleArray knotSequences;
	for (unsigned i = 0; i < nknots; ++i)
	{ 
		knotSequences.append(static_cast<double>(i));
	}
	controlVertices.append(MPoint(0., 0., 0.));
	controlVertices.append(MPoint(1., 1., 1.));
	MFnNurbsCurve  curveFn;
	MStatus stat;
	MObject curve = curveFn.create(controlVertices, knotSequences, deg,
		MFnNurbsCurve::kOpen, false, false, MObject::kNullObj, &stat);
	MString name = curveFn.name();
	MGlobal::displayInfo(name);
	if (stat != MS::kSuccess)
	{
		MGlobal::displayInfo("curve creation failed");
	}
	// message in scriptor editor
	MGlobal::executeCommand(MString("extrude - ch true - rn false - po 1 - et 2 "
		"- ucp 1 - fpt 1 - upn 1 - rotation 0" 
		"- scale 1 - rsp 1 ") + "nurbsCircle1 " + name, true);
```

### b.  Parse Command Line arguments
Added command line arguments.  This was implemented in a procedure in LSystemCmd
entitled `parseArguments()` that has parses the iteration, the step, the
grammar, and the angle. One example is as follows:

```
			if (MString("-step") == args.asString(i, &status) && MS::kSuccess == status) {
				x = args.asDouble(++i, &status);
				if (MS::kSuccess == status)
				{
					step = MDistance(x, MDistance::Unit::kCentimeters);
				}
			}
```
Where the step creates an MDistance object that can easily be converted into
several different units.
### c. Create a Gui.

This is implemented in the script `LSystemGui.mel` .  This creates a window
called `myWindow` with Sliders and buttons.  A picture of the Gui is:



![Gui](https://github.com/akhabbaz/MayaPluginHW2/blob/master/InputScreenV1.PNG)
This has a browse button, an iteration, and angle and a step.  The browse button
loads grammars from files. The iteration is really the maximum iteration.  For
example if iteration is 4 then as the frames advance, the frames can be 0, 1, 2,
3 and the cycle repeats. The angle is in degrees and the step is in centimeters.
In addition there is a scroll field that accepts grammars as input. Once the
grammar is inputted and run is pressed, the current value as seen in me mel gui
is displayed in the scroll field.  This is a check.  These variables are read
correctly in the C++ command line because a dialog box generated by the C++ code
in parseArguments produces the correct variables.

There is also a shelf in maya that runs the main scripts the code for that shelf
is:
```
print("User defined macro");
unloadPlugin LSystemMaya;
loadPlugin LSystemMaya;
source LSystemGui
```
This just unloads the plugin, reloads it and runs the Mel script for the dialog
Gui.  

##2.3

### a.  Implement an LSystemNode derived from MPxNode.

The L-SystemNode that I created was based on animCubeNode.  This class is called
`LSystemNode` and it implements a constructor, destructor and the creator.  The
id is chosen to be in the allowable range for Maya.  The initialization routine
creates the following attributes that are part of this node.

*Time (MFnUnitAttribute)
*StepSize (MFnNumericAttribute)
*angle    (MFnNumericAttribute)
*iteration(MFnNumericAttribute)
*grammar  (MFnTypedAttribute)
*outputMesh (MFnTypedAttribute)

The grammar was the hardest to implement because it had to be allocated first
using MFnStringData.  the LSystemNode was registered so createNode worked.  All
these variables were considered input and so the output mesh depended on each
one of this input attributes.  Typically Time changes (as frame number).  Also
in initialize an initial value of all these variables is chosen but that does
not depend at all on the values set in the Gui.  These values are set at
initialization.  We want the gui to modify these variables and cause the compute
funtion to be called.  Here the attributes are all added and attributeAffects is
called so that if any input attribute changes and the output mesh is needed it
is recomputed.  See step c shows how these input attributes ar connected to the
Gui.


###b.  Implementing the compute function.

The compute function essentially runs the `LSystem` command `process` that
converts a grammar string int a series of branches.  The LSystem is stored as a
private member called `Connections`.  The first call to compute must initialize
`Connections` and that is done in a private function called `updateConnection`.
That initializes the LSystem with the LSystem string, and sets the angle.  These
do not change for the entire animation.  Boolean `firstTime` then gets set to
false.  Each iteration a member function `updateStepSize` is called and this
computes the frameNumber, and updates the step size.  There are iteration number
of different trees. As time goes on the iterations repeat.  The step size
decreases with iteration number so the overall size of the shape does not change
with iteration. 

The heart of the compute function is in  `createLSystemMesh`.  This processes
the iteration number, and for each branch, it creates a cylinder of the same
radius to length ratio, positioned according to the branch points.

some examples are 

![Tree Example Frame 3 and Cylinder](https://github.com/akhabbaz/MayaPluginHW2/blob/master/TreeFrame3.PNG)
This examle is shows the 3rd frame of a tree structure where each original leg
becomes three legs.  The number of segments triples each iteration but because
of symmetry, some of the segments that appear as one really are two segments.

![HW example grammar frame 3](https://github.com/akhabbaz/MayaPluginHW2/blob/master/Example%20Grammar%20Frame%203.PNG)
This example uses the grammar rule in the homework on the 3rd frame.  This looks
exactly like in the assignment description.  The rule used is to the right.

![Sample1_8Iterations](https://github.com/akhabbaz/MayaPluginHW2/blob/master/Sample1_8Iterations.PNG) 
Here are 8 iterations from the grammar rule loaded frome Sample1 using the
browse Button.


### c.
To connect this attributes to the LSystemGui functions, I wanted to use the
variables i set previously in the Gui.  Since the variables were already in the
C++ code the cleanest way to get the attributes into the Lsystem node was to
create the needed Nodes in C++ and set the attributes for this particular node.

We already implemented parseArguments to parse the commands, so logically we
should not have to input the data again.  We also should not have to resort to
reusing the Mel script that has the attribute values.

This was implemented in `LSystemCmd.cpp`.  The first member function is
`createNode()`.  That uses MFnDagModifier to create Directed acyclic graph
nodes, transform and a Mesh node.
The LSystemNode is another node created and that is a dependency graph node that
does not have geometry directly.  This node is refered to by its type ID.  Next
I find the plug for this node based on the plug's name such as "stepSize",
"angle" and "grammar", and "maxIteration".  If the plug was not found the status indicator would be
false.  Here they were all true.  Grammar posed the biggest challenge because
the plug in was not being found that eventually was traced to a bug in the
initialization routine.  This setting was verified to be correct and was checked
in the compute function using MHandle to get the data based on its attribute
name and getting the value.  These values agreed with those set in the Gui.

This was a clean way to implement the node.  It was done without needing to add
another button to the GUi.


##Summary

This example demonstrates getting arguments into a C++ function, setting up
attributes, creating nodes, and setting up an LSystem.  It works really well.  




1. The Algorithmic Beauty of Plants" by Przemyslaw Prusinkiewicz and Arstid Lindenmeyer
is available free at http://algorithmicbotany.org/papers/#abop
