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
  AlsaOutput( const std::string &pcmName = "default:0" ) throw (Error);
  virtual ~AlsaOutput(void);
  void close(void);
  void write( SequencePtr sequence ) throw (Error);
  static VALUE cRubyClass;
  static VALUE registerRubyClass( VALUE rbModule );
  static void deleteRubyObject( void *ptr );
  static VALUE wrapNew( VALUE rbClass, VALUE rbPCMName );
  static VALUE wrapClose( VALUE rbSelf );
  static VALUE wrapWrite( VALUE rbSelf, VALUE rbSequence );
protected:
  snd_pcm_t *m_pcmHandle;
  std::string m_pcmName;
};

typedef boost::shared_ptr< AlsaOutput > AlsaOutputPtr;

#endif

