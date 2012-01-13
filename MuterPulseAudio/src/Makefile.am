NULL =

lib_LTLIBRARIES=libMuterPulseAudio.la

include_HEADERS = MuterPulseAudio.h

libMuterPulseAudio_la_SOURCES= \
	MuterPulseAudio.cpp \
	AudioMixerControl.h \
	AudioMixerControl.cpp \
	SinkInput.h \
	SinkInput.cpp \
	$(NULL)
	
libMuterPulseAudio_la_LDFLAGS= -module -avoid-version 
libMuterPulseAudio_la_LIBADD = \
	$(PULSEAUDIO_LIBS)\
	$(NULL)

bin_PROGRAMS = TestAudioMixerControl

TestAudioMixerControl_SOURCES = \
	TestAudioMixerControl.cpp \
	$(NULL)

TestAudioMixerControl_LDADD = \
	AudioMixerControl.o\
	SinkInput.o \
	$(PULSEAUDIO_LIBS)\
	$(NULL)

AM_CPPFLAGS = \
	$(PULSEAUDIO_CFLAGS) \
	$(NULL)
