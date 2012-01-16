/* 
 * AudioGraph Audio Graph Layer for Jamoma DSP
 * Creates a wrapper for TTAudioObjects that can be used to build an audio processing graph.
 * Copyright © 2008, Timothy Place
 * 
 * License: This code is licensed under the terms of the "New BSD License"
 * http://creativecommons.org/licenses/BSD/
 */

#include "TTAudioGraphObject.h"
#include "TTAudioGraphInlet.h"		// required for windows build
#include "TTAudioGraphInput.h"
#include "TTAudioEngine.h"

#define thisTTClass			TTAudioGraphInput
#define thisTTClassName		"adc"
#define thisTTClassTags		"audio, graph, input"

TT_AUDIO_CONSTRUCTOR
{
	mAudioEngine = TTAudioEngine::create();
	mBuffer = (TTAudioEnginePtr(mAudioEngine))->TTAudioEngineGetInputSignalReference();
	
	addAttributeWithGetterAndSetter(SampleRate, kTypeUInt32);
	addAttributeWithGetterAndSetter(VectorSize, kTypeUInt16);
	addAttributeWithGetterAndSetter(Device,		kTypeSymbol);

	addMessage(start);
	addMessage(stop);
	addMessageWithArguments(getAvailableDeviceNames);
	
	setProcessMethod(processAudio);
	
	setAttributeValue(kTTSym_sampleRate, 44100);
	setAttributeValue(kTTSym_vectorSize, 512);
}


TTAudioGraphInput::~TTAudioGraphInput()
{
	TTAudioEngine::destroy();
	TTObjectRelease(&mBuffer);
}


TTErr TTAudioGraphInput::getAvailableDeviceNames(const TTValue&, TTValue& returnedDeviceNames)
{
	return mAudioEngine->sendMessage(TT("getAvailableInputDeviceNames"), kTTValNONE, returnedDeviceNames);
}


TTErr TTAudioGraphInput::start()
{
	mAudioEngine->sendMessage(TT("start"));
	return kTTErrNone;
}


TTErr TTAudioGraphInput::stop()
{
	mAudioEngine->sendMessage(TT("stop"));
	return kTTErrNone;
}

TTErr TTAudioGraphInput::setSampleRate(const TTValue& newValue)
{
	return mAudioEngine->setAttributeValue(kTTSym_sampleRate, const_cast<TTValue&>(newValue));
}

TTErr TTAudioGraphInput::getSampleRate(TTValue& returnedValue)
{
	return mAudioEngine->getAttributeValue(kTTSym_sampleRate, returnedValue);
}


TTErr TTAudioGraphInput::setVectorSize(const TTValue& newValue)
{
	return mAudioEngine->setAttributeValue(kTTSym_vectorSize, const_cast<TTValue&>(newValue));
}

TTErr TTAudioGraphInput::getVectorSize(TTValue& returnedValue)
{
	return mAudioEngine->getAttributeValue(kTTSym_vectorSize, returnedValue);
}


TTErr TTAudioGraphInput::setDevice(const TTValue& newValue)
{
	return mAudioEngine->setAttributeValue(TT("inputDevice"), const_cast<TTValue&>(newValue));
}

TTErr TTAudioGraphInput::getDevice(TTValue& returnedValue)
{
	return mAudioEngine->getAttributeValue(TT("inputDevice"), returnedValue);
}


TTErr TTAudioGraphInput::processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
{
	if (outputs->numAudioSignals){
		TTAudioSignal&	out = outputs->getSignal(0);

		out = *mBuffer;
		return kTTErrNone;
	}
	else
		return kTTErrBadChannelConfig;
}
