/** @file
 *
 * @ingroup audioGraphLibrary
 *
 * @brief This object represents a single 'outlet' from a TTAudioGraphObject.
 *
 * @details TTAudioGraphObject maintains a vector of these outlets.
 *
 * @authors Timothy Place, Trond Lossius
 *
 * @copyright Copyright © 2010, Timothy Place @n
 * This code is licensed under the terms of the "New BSD License" @n
 * http://creativecommons.org/licenses/BSD/
 */


#ifndef __TTAUDIOGRAPH_OUTLET_H__
#define __TTAUDIOGRAPH_OUTLET_H__

#include "TTAudioGraph.h"


/******************************************************************************************/

/**	This object represents a single 'outlet' from a TTAudioGraphObject.
	TTAudioGraphObject maintains a vector of these outlets.
*/
class TTAudioGraphOutlet {
	friend class TTAudioGraphObject;
	
protected:	
	
	TTAudioSignalPtr		mBufferedOutput;			///< Output vector from the audio graph node
	
public:

	/** Constructor.
	 */
	TTAudioGraphOutlet() : 
		mBufferedOutput(NULL)
	{
		TTObjectInstantiate(kTTSym_audiosignal, &mBufferedOutput, 1);
	}
	
	
	/** destructor.
	 */
	~TTAudioGraphOutlet()
	{
		TTObjectRelease(&mBufferedOutput);
	}
	
	
	// Copying Functions are critical due to use by std::vector 
	
	TTAudioGraphOutlet(const TTAudioGraphOutlet& original) : 
		mBufferedOutput(NULL)
	{
//		mBufferedOutput = TTObjectReference(original.mBufferedOutput);
		TTObjectInstantiate(kTTSym_audiosignal, &mBufferedOutput, 1);
	}
	
	
	TTAudioGraphOutlet& operator=(const TTAudioGraphOutlet& source)
	{
		TTObjectRelease(&mBufferedOutput);
		mBufferedOutput = TTObjectReference(source.mBufferedOutput);
		return *this;
	}
	

	// Audio Signal Access Methods

	TTUInt16 getNumOutputChannels()
	{
		return mBufferedOutput->getNumChannelsAsInt();
	}
	
	
	TTUInt16 getOutputVectorSize()
	{
		return mBufferedOutput->getVectorSizeAsInt();
	}

	
	TTAudioSignalPtr getBuffer()
	{
		return mBufferedOutput;
	}
	
};




#endif // __TTAUDIOGRAPH_OUTLET_H__
