/* HornetsEye - Computer Vision with Ruby
   Copyright (C) 2012   Jan Wedekind

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
#ifndef ALSAINPUT_HH
#define ALSAINPUT_HH

#include <alsa/asoundlib.h>
#include <string>
#include "rubyinc.hh"
#include "error.hh"
#include "sequence.hh"

class AlsaInput
{
public:
  AlsaInput( const std::string &pcmName = "default:0",
             unsigned int rate = 48000, unsigned int channels = 2) throw (Error);
  virtual ~AlsaInput(void);
  void close(void);
  SequencePtr read( int samples ) throw (Error);
  void drop(void) throw (Error);
  unsigned int rate(void);
  unsigned int channels(void);
  int avail(void) throw (Error);
  void lock(void);
  void unlock(void);
  void prepare(void) throw (Error);
  static VALUE cRubyClass;
  static VALUE registerRubyClass( VALUE rbModule );
  static void deleteRubyObject( void *ptr );
  static VALUE wrapNew(VALUE rbClass, VALUE rbPCMName, VALUE rbRate,
                       VALUE rbChannels);
  static VALUE wrapClose( VALUE rbSelf );
  static VALUE wrapRead( VALUE rbSelf, VALUE rbSamples );
  static VALUE wrapRate( VALUE rbSelf );
  static VALUE wrapChannels( VALUE rbSelf );
  static VALUE wrapAvail( VALUE rbSelf );
  static VALUE wrapPrepare( VALUE rbSelf );
protected:
  void readi(short int *data, int count);
  void threadFunc(void);
  static void *staticThreadFunc( void *self );
  snd_pcm_t *m_pcmHandle;
  std::string m_pcmName;
  unsigned int m_rate;
  unsigned int m_channels;
  snd_pcm_uframes_t m_periodSize;
  bool m_threadInitialised;
  boost::shared_array<short int> m_data;
  int m_start;
  int m_count;
  int m_size;
  pthread_t m_thread;
  pthread_mutex_t m_mutex;
};

typedef boost::shared_ptr< AlsaInput > AlsaInputPtr;

#endif

