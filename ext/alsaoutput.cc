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

AlsaOutput::AlsaOutput( const string &pcmName, unsigned int rate,
                        int channels, int periods, int periodSize ) throw (Error):
  m_pcmHandle(NULL), m_pcmName( pcmName ), m_rate( rate )
{
  try {
    snd_pcm_hw_params_t *hwParams;
    snd_pcm_hw_params_alloca( &hwParams );
    int err = snd_pcm_open( &m_pcmHandle, m_pcmName.c_str(), SND_PCM_STREAM_PLAYBACK,
                            SND_PCM_NONBLOCK );
    ERRORMACRO( err >= 0, Error, , "Error opening PCM device \"" << m_pcmName
                << "\": " << snd_strerror( err ) );
    err = snd_pcm_hw_params_any( m_pcmHandle, hwParams );
    ERRORMACRO( err >= 0, Error, , "Unable to configure the PCM device \""
                << m_pcmName << "\": " << snd_strerror( err ) );
    err = snd_pcm_hw_params_set_access( m_pcmHandle, hwParams,
                                        SND_PCM_ACCESS_RW_INTERLEAVED );
    ERRORMACRO( err >= 0, Error, , "Error setting PCM device \""
                << m_pcmName << "\" to interlaced access: " << snd_strerror( err ) );
    err = snd_pcm_hw_params_set_format( m_pcmHandle, hwParams,
                                        SND_PCM_FORMAT_S16_LE );
    ERRORMACRO( err >= 0, Error, , "Error setting PCM device \"" << m_pcmName
                << "\" to 16-bit signed integer format: " << snd_strerror( err ) );
    err = snd_pcm_hw_params_set_rate_near( m_pcmHandle, hwParams, &m_rate, 0 );
    ERRORMACRO( err >= 0, Error, , "Error setting sampling rate of PCM device \""
                << m_pcmName << "\" to " << rate << " Hz: " << snd_strerror( err ) );
    err = snd_pcm_hw_params_set_channels( m_pcmHandle, hwParams, channels );
    ERRORMACRO( err >= 0, Error, , "Error setting number of channels of PCM device \""
                << m_pcmName << "\" to " << channels << ": " << snd_strerror( err ) );
    err = snd_pcm_hw_params_set_periods( m_pcmHandle, hwParams, periods, 0 );
    ERRORMACRO( err >= 0, Error, , "Error setting number of periods of PCM device \""
                << m_pcmName << "\" to " << periods << ": " << snd_strerror( err ) );
    snd_pcm_uframes_t bufSize = periodSize * periods / (channels * 2 );
    err = snd_pcm_hw_params_set_buffer_size_near( m_pcmHandle, hwParams, &bufSize );
    ERRORMACRO( err >= 0, Error, , "Error setting buffer size of PCM device \""
                << m_pcmName << "\" to " << bufSize << " frames: "
                << snd_strerror( err ) );
    err = snd_pcm_hw_params( m_pcmHandle, hwParams );
    ERRORMACRO( err >= 0, Error, , "Error setting parameters of PCM device \""
                << m_pcmName << "\": " << snd_strerror( err ) );
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

unsigned int AlsaOutput::rate(void)
{
  return m_rate;
}

VALUE AlsaOutput::registerRubyClass( VALUE rbModule )
{
  cRubyClass = rb_define_class_under( rbModule, "AlsaOutput", rb_cObject );
  rb_define_singleton_method( cRubyClass, "new",
                              RUBY_METHOD_FUNC( wrapNew ), 5 );
  rb_define_method( cRubyClass, "close", RUBY_METHOD_FUNC( wrapClose ), 0 );
  rb_define_method( cRubyClass, "write", RUBY_METHOD_FUNC( wrapWrite ), 1 );
  rb_define_method( cRubyClass, "rate", RUBY_METHOD_FUNC( wrapRate ), 0 );
}

void AlsaOutput::deleteRubyObject( void *ptr )
{
  delete (AlsaOutputPtr *)ptr;
}

VALUE AlsaOutput::wrapNew( VALUE rbClass, VALUE rbPCMName, VALUE rbRate,
                           VALUE rbChannels, VALUE rbPeriods, VALUE rbPeriodSize )
{
  VALUE retVal = Qnil;
  try {
    rb_check_type( rbPCMName, T_STRING );
    AlsaOutputPtr ptr( new AlsaOutput( StringValuePtr( rbPCMName ),
                                       NUM2UINT( rbRate ), NUM2INT( rbChannels ),
                                       NUM2INT( rbPeriods ),
                                       NUM2INT( rbPeriodSize ) ) );
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

VALUE AlsaOutput::wrapRate( VALUE rbSelf )
{
  AlsaOutputPtr *self; Data_Get_Struct( rbSelf, AlsaOutputPtr, self );
  return UINT2NUM( (*self)->rate() );
}

