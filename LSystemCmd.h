#ifndef CreateLSystemCmd_H_
#define CreateLSystemCmd_H_

#include <maya/MPxCommand.h>
#include <string>
#include <maya/MDistance.h>
#include <maya/MAngle.h>
#include <maya/MObject.h>

class LSystemCmd : public MPxCommand
{
public:
    LSystemCmd();
    virtual ~LSystemCmd();
    static void* creator() { return new LSystemCmd(); }
    MStatus doIt( const MArgList& args );
private:
    MStatus parseArguments(const MArgList& args);
    // step size
    MDistance step;
    // angle in degrees
    MAngle angle;
    //  grammar 
    MString grammar;
    // number of iterations
    int iter;
	// LSystem Node used to build the tree
	MObject LSnode;
	MStatus CreateNode() const;
	// adds a new grammar attribute with the defined grammar
	MStatus addGrammarAttribute(MObject obj) const;
	// update the attributes of the LSystem Node LSNode created via MelScript.
	MStatus updateLSystemNodeAttributes(MObject LSNode) const;
	//connects the LSystemNode to the shape and time nodes
	MStatus connectNode(MObject LSNode, MObject meshNode) const;
	MStatus connectGrammarAttribute(MObject LSNode, MPlug gplug) const;
};

#endif
