/* 
 *	dac≈
 *	Jamoma Multicore external object for Max
 *	Copyright © 2008 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the GNU LGPL
 *	http://www.gnu.org/licenses/lgpl.html 
 */

#include "maxMulticore.h"


// Data Structure for this object
struct Dac {
    t_object				obj;
	TTMulticoreObjectPtr	multicoreObject;
};
typedef Dac* DacPtr;


// Prototypes for methods
DacPtr	DacNew(SymbolPtr msg, AtomCount argc, AtomPtr argv);
void	DacFree(DacPtr self);
void	DacAssist(DacPtr self, void* b, long msg, long arg, char* dst);
TTErr	DacReset(DacPtr self);
TTErr	DacConnect(DacPtr self, TTMulticoreObjectPtr audioSourceObject, long sourceOutletNumber);
TTErr	DacStart(DacPtr self);
TTErr	DacStop(DacPtr self);
// Prototypes for attribute accessors
MaxErr	DacSetSampleRate(DacPtr self, void* attr, AtomCount argc, AtomPtr argv);
MaxErr	DacGetSampleRate(DacPtr self, void* attr, AtomCount* argc, AtomPtr* argv);
MaxErr	DacSetVectorSize(DacPtr self, void* attr, AtomCount argc, AtomPtr argv);
MaxErr	DacGetVectorSize(DacPtr self, void* attr, AtomCount* argc, AtomPtr* argv);


// Globals
static ClassPtr sDacClass;


/************************************************************************************/
// Main() Function

int main(void)
{
	ClassPtr c;
	
	TTMulticoreInit();	
	common_symbols_init();
	
	c = class_new("jcom.dac≈", (method)DacNew, (method)DacFree, sizeof(Dac), (method)0L, A_GIMME, 0);
	
	class_addmethod(c, (method)DacStart,		"start",		0);
	class_addmethod(c, (method)DacStop,		"stop",			0);
	//class_addmethod(c, (method)DacNotify,		"notify",		A_CANT, 0);
	class_addmethod(c, (method)DacReset,		"multicore.reset",	A_CANT, 0);
	class_addmethod(c, (method)DacConnect,		"multicore.connect",	A_OBJ, A_LONG, 0);
	class_addmethod(c, (method)DacAssist,		"assist",		A_CANT, 0); 
	class_addmethod(c, (method)object_obex_dumpout,	"dumpout",		A_CANT, 0);  
	
	CLASS_ATTR_LONG(c,		"sampleRate",	0,		Dac,	obj);
	CLASS_ATTR_ACCESSORS(c,	"sampleRate",	DacGetSampleRate,	DacSetSampleRate);
	
	CLASS_ATTR_LONG(c,		"vectorSize",	0,		Dac,	obj);
	CLASS_ATTR_ACCESSORS(c,	"vectorSize",	DacGetVectorSize,	DacSetVectorSize);
	
	class_register(_sym_box, c);
	sDacClass = c;
	return 0;
}


/************************************************************************************/
// Life Cycle

DacPtr DacNew(SymbolPtr msg, AtomCount argc, AtomPtr argv)
{
    DacPtr		self = DacPtr(object_alloc(sDacClass));
	TTValue		v;
	TTErr		err;
	
    if (self) {
		v.setSize(2);
		v.set(0, TT("multicore.output"));
		v.set(1, 2);
		err = TTObjectInstantiate(TT("multicore.object"), (TTObjectPtr*)&self->multicoreObject, v);

		v = TTPtr(self->multicoreObject);
		//self->multicoreObject->mUnitGenerator->sendMessage(TT("setOwner"), v);

		attr_args_process(self, argc, argv);
		object_obex_store((void*)self, _sym_dumpout, (object*)outlet_new(self, NULL));
	}
	return self;
}


void DacFree(DacPtr self)
{
	TTObjectRelease((TTObjectPtr*)&self->multicoreObject);
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void DacAssist(DacPtr self, void* b, long msg, long arg, char* dst)
{
	if (msg==1)				// Inlets
		strcpy(dst, "multichannel audio connection and control messages");		
	else if (msg==2)		// Outlets
		strcpy(dst, "dumpout");
}


TTErr DacReset(DacPtr self)
{
	return self->multicoreObject->reset();
}


TTErr DacConnect(DacPtr self, TTMulticoreObjectPtr audioSourceObject, long sourceOutletNumber)
{
	return self->multicoreObject->connect(audioSourceObject, sourceOutletNumber);
}


TTErr DacStart(DacPtr self)
{
	MaxErr		err;
	ObjectPtr	patcher = NULL;
	ObjectPtr	box = NULL;
	ObjectPtr	o = NULL;
	method		multicoreSetupMethod = NULL;
	long		vectorSize;
	
	self->multicoreObject->mUnitGenerator->getAttributeValue(TT("vectorSize"), vectorSize);
	
	err = object_obex_lookup(self, gensym("#P"), &patcher);
	box = jpatcher_get_firstobject(patcher);
	while (box) {
		o = jbox_get_object(box);
		multicoreSetupMethod = zgetfn(o, gensym("multicore.reset"));
		if (multicoreSetupMethod)
			err = (MaxErr)multicoreSetupMethod(o, vectorSize);
		box = jbox_get_nextobject(box);
	}
	box = jpatcher_get_firstobject(patcher);
	while (box) {
		o = jbox_get_object(box);
		multicoreSetupMethod = zgetfn(o, gensym("multicore.setup"));
		if (multicoreSetupMethod)
			err = (MaxErr)multicoreSetupMethod(o);
		box = jbox_get_nextobject(box);
	}
	
	TTMulticoreInitData	initData;
	initData.vectorSize = vectorSize;
	self->multicoreObject->init(initData);
	
	return self->multicoreObject->mUnitGenerator->sendMessage(TT("start"));
}


TTErr DacStop(DacPtr self)
{	
	return self->multicoreObject->mUnitGenerator->sendMessage(TT("stop"));
}



MaxErr DacSetSampleRate(DacPtr self, void* attr, AtomCount argc, AtomPtr argv)
{
	if (argc) {
		TTUInt32 sr = atom_getlong(argv);
		self->multicoreObject->mUnitGenerator->setAttributeValue(TT("sampleRate"), sr);
	}
	return MAX_ERR_NONE;
}

MaxErr DacGetSampleRate(DacPtr self, void* attr, AtomCount* argc, AtomPtr* argv)
{
	long sr;
	
	self->multicoreObject->mUnitGenerator->getAttributeValue(TT("sampleRate"), sr);
	
	*argc = 1;
	if (!(*argv)) // otherwise use memory passed in
		*argv = (t_atom *)sysmem_newptr(sizeof(t_atom));
	atom_setlong(*argv, sr);
	return MAX_ERR_NONE;
}


MaxErr DacSetVectorSize(DacPtr self, void* attr, AtomCount argc, AtomPtr argv)
{
	if (argc) {
		TTUInt32 vs = atom_getlong(argv);
		self->multicoreObject->mUnitGenerator->setAttributeValue(TT("vectorSize"), vs);
	}
	return MAX_ERR_NONE;
}

MaxErr DacGetVectorSize(DacPtr self, void* attr, AtomCount* argc, AtomPtr* argv)
{
	long vs;
	
	self->multicoreObject->mUnitGenerator->getAttributeValue(TT("vectorSize"), vs);
	
	*argc = 1;
	if (!(*argv)) // otherwise use memory passed in
		*argv = (t_atom *)sysmem_newptr(sizeof(t_atom));
	atom_setlong(*argv, vs);
	return MAX_ERR_NONE;
}


