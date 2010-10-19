/* HornetsEye - Computer Vision with Ruby
   Copyright (C) 2006, 2007, 2008, 2009, 2010   Jan Wedekind

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */
#ifndef ALSAOUTPUT_HH
#define ALSAOUTPUT_HH

#include <alsa/asoundlib.h>
#include <string>
#include "rubyinc.hh"
#include "error.hh"
#include "sequence.hh"

class AlsaOutput
{
public:
  AlsaOutput( const std::string &pcmName = "default:0",
              unsigned int rate = 48000, unsigned int channels = 2,
              int periods = 2, int frames = 1024 ) throw (Error);
  virtual ~AlsaOutput(void);
  void close(void);
  void write( SequencePtr sequence ) throw (Error);
  void drop(void) throw (Error);
  void drain(void) throw (Error);
  unsigned int rate(void);
  int availUpdate(void) throw (Error);
  static VALUE cRubyClass;
  static VALUE registerRubyClass( VALUE rbModule );
  static void deleteRubyObject( void *ptr );
  static VALUE wrapNew( VALUE rbClass, VALUE rbPCMName, VALUE rbRate,
                        VALUE rbChannels, VALUE rbPeriods, VALUE rbFrames );
  static VALUE wrapClose( VALUE rbSelf );
  static VALUE wrapWrite( VALUE rbSelf, VALUE rbSequence );
  static VALUE wrapDrop( VALUE rbSelf );
  static VALUE wrapDrain( VALUE rbSelf );
  static VALUE wrapRate( VALUE rbSelf );
  static VALUE wrapAvailUpdate( VALUE rbSelf );
protected:
  snd_pcm_t *m_pcmHandle;
  std::string m_pcmName;
  unsigned int m_rate;
};

typedef boost::shared_ptr< AlsaOutput > AlsaOutputPtr;

#endif

