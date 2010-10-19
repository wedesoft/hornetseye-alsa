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
#include "alsaoutput.hh"

using namespace std;

VALUE AlsaOutput::cRubyClass = Qnil;

AlsaOutput::AlsaOutput( const string &pcmName ) throw (Error):
  m_pcmHandle(NULL), m_pcmName( pcmName )
{
  try {
    snd_pcm_hw_params_t *hwParams;
    snd_pcm_hw_params_alloca( &hwParams );
    int err = snd_pcm_open( &m_pcmHandle, m_pcmName.c_str(), SND_PCM_STREAM_PLAYBACK,
                            SND_PCM_NONBLOCK );
    ERRORMACRO( err >= 0, Error, , "Error opening PCM device " << m_pcmName << ": "
                << snd_strerror(err) );
  } catch ( Error &e ) {
    close();
    throw e;
  };
}

AlsaOutput::~AlsaOutput(void)
{
  close();
}

void AlsaOutput::close(void)
{
  if ( m_pcmHandle != NULL ) {
    snd_pcm_close( m_pcmHandle );
    m_pcmHandle = NULL;
  };
}

void AlsaOutput::write( SequencePtr frame ) throw (Error)
{
}

VALUE AlsaOutput::registerRubyClass( VALUE rbModule )
{
  cRubyClass = rb_define_class_under( rbModule, "AlsaOutput", rb_cObject );
  rb_define_singleton_method( cRubyClass, "new",
                              RUBY_METHOD_FUNC( wrapNew ), 1 );
  rb_define_method( cRubyClass, "close", RUBY_METHOD_FUNC( wrapClose ), 0 );
  rb_define_method( cRubyClass, "write", RUBY_METHOD_FUNC( wrapWrite ), 1 );
}

void AlsaOutput::deleteRubyObject( void *ptr )
{
  delete (AlsaOutputPtr *)ptr;
}

VALUE AlsaOutput::wrapNew( VALUE rbClass, VALUE rbPCMName )
{
  VALUE retVal = Qnil;
  try {
    rb_check_type( rbPCMName, T_STRING );
    AlsaOutputPtr ptr( new AlsaOutput( StringValuePtr( rbPCMName ) ) );
    retVal = Data_Wrap_Struct( rbClass, 0, deleteRubyObject,
                               new AlsaOutputPtr( ptr ) );
  } catch ( exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
}

VALUE AlsaOutput::wrapClose( VALUE rbSelf )
{
  AlsaOutputPtr *self; Data_Get_Struct( rbSelf, AlsaOutputPtr, self );
  (*self)->close();
  return rbSelf;
}

VALUE AlsaOutput::wrapWrite( VALUE rbSelf, VALUE rbSequence )
{
  try {
    AlsaOutputPtr *self; Data_Get_Struct( rbSelf, AlsaOutputPtr, self );
    SequencePtr sequence( new Sequence( rbSequence ) );
    (*self)->write( sequence );
  } catch ( exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbSequence;
}

