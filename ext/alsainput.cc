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
#include "alsainput.hh"

using namespace std;

VALUE AlsaInput::cRubyClass = Qnil;

AlsaInput::AlsaInput( const string &pcmName, unsigned int rate,
                      unsigned int channels, int periods,
                      snd_pcm_uframes_t frames ) throw (Error):
  m_pcmHandle(NULL), m_pcmName( pcmName ), m_rate( rate ), m_channels( channels )
{
  try {
    snd_pcm_hw_params_t *hwParams;
    snd_pcm_hw_params_alloca( &hwParams );
    int err = snd_pcm_open( &m_pcmHandle, m_pcmName.c_str(), SND_PCM_STREAM_CAPTURE,
                            0 );
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
    err = snd_pcm_hw_params_set_buffer_size_near( m_pcmHandle, hwParams, &frames );
    ERRORMACRO( err >= 0, Error, , "Error setting buffer size of PCM device \""
                << m_pcmName << "\" to " << frames << " frames: "
                << snd_strerror( err ) );
    err = snd_pcm_hw_params( m_pcmHandle, hwParams );
    ERRORMACRO( err >= 0, Error, , "Error setting parameters of PCM device \""
                << m_pcmName << "\": " << snd_strerror( err ) );
  } catch ( Error &e ) {
    close();
    throw e;
  };
}

AlsaInput::~AlsaInput(void)
{
  close();
}

void AlsaInput::close(void)
{
  if ( m_pcmHandle != NULL ) {
    // drop();
    snd_pcm_close( m_pcmHandle );
    m_pcmHandle = NULL;
  };
}

SequencePtr AlsaInput::read( int samples ) throw (Error)
{
  ERRORMACRO( m_pcmHandle != NULL, Error, , "PCM device \"" << m_pcmName
              << "\" is not open. Did you call \"close\" before?" );
  SequencePtr frame( new Sequence( (int)( samples * 2 * m_channels ) ) );
  int err;
  while ( ( err = snd_pcm_readi( m_pcmHandle, (short int *)frame->data(),
                                 samples ) ) < 0 ) {
    err = snd_pcm_recover( m_pcmHandle, err, 1 );
    ERRORMACRO( err >= 0, Error, , "Error reading audio frames from PCM device \""
                << m_pcmName << "\": " << snd_strerror( err ) );
  };
  ERRORMACRO( samples == err, Error, , "Only managed to read " << err << " of "
              << samples << " frames from PCM device \"" << m_pcmName << "\"" );
  return frame;
}

unsigned int AlsaInput::rate(void)
{
  return m_rate;
}

unsigned int AlsaInput::channels(void)
{
  return m_channels;
}

void AlsaInput::prepare(void) throw (Error)
{
  ERRORMACRO( m_pcmHandle != NULL, Error, , "PCM device \"" << m_pcmName
              << "\" is not open. Did you call \"close\" before?" );
  int err = snd_pcm_prepare( m_pcmHandle );
  ERRORMACRO( err >= 0, Error, , "Error preparing PCM device \"" << m_pcmName
              << "\": " << snd_strerror( err ) );
}

VALUE AlsaInput::registerRubyClass( VALUE rbModule )
{
  cRubyClass = rb_define_class_under( rbModule, "AlsaInput", rb_cObject );
  rb_define_singleton_method( cRubyClass, "new",
                              RUBY_METHOD_FUNC( wrapNew ), 5 );
  rb_define_method( cRubyClass, "close", RUBY_METHOD_FUNC( wrapClose ), 0 );
  rb_define_method( cRubyClass, "read", RUBY_METHOD_FUNC( wrapRead ), 1 );
  rb_define_method( cRubyClass, "rate", RUBY_METHOD_FUNC( wrapRate ), 0 );
  rb_define_method( cRubyClass, "channels", RUBY_METHOD_FUNC( wrapChannels ), 0 );
  rb_define_method( cRubyClass, "prepare", RUBY_METHOD_FUNC( wrapPrepare ), 0 );
}

void AlsaInput::deleteRubyObject( void *ptr )
{
  delete (AlsaInputPtr *)ptr;
}

VALUE AlsaInput::wrapNew( VALUE rbClass, VALUE rbPCMName, VALUE rbRate,
                          VALUE rbChannels, VALUE rbPeriods, VALUE rbFrames )
{
  VALUE retVal = Qnil;
  try {
    rb_check_type( rbPCMName, T_STRING );
    AlsaInputPtr ptr( new AlsaInput( StringValuePtr( rbPCMName ),
                                     NUM2UINT( rbRate ), NUM2UINT( rbChannels ),
                                     NUM2INT( rbPeriods ), NUM2INT( rbFrames ) ) );
    retVal = Data_Wrap_Struct( rbClass, 0, deleteRubyObject,
                               new AlsaInputPtr( ptr ) );
  } catch ( exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
}

VALUE AlsaInput::wrapClose( VALUE rbSelf )
{
  AlsaInputPtr *self; Data_Get_Struct( rbSelf, AlsaInputPtr, self );
  (*self)->close();
  return rbSelf;
}

VALUE AlsaInput::wrapRead( VALUE rbSelf, VALUE rbSamples )
{
  VALUE rbRetVal = Qnil;
  try {
    AlsaInputPtr *self; Data_Get_Struct( rbSelf, AlsaInputPtr, self );
    SequencePtr sequence( (*self)->read( NUM2INT( rbSamples ) ) );
    rbRetVal = sequence->rubyObject();
  } catch ( exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbRetVal;
}

VALUE AlsaInput::wrapRate( VALUE rbSelf )
{
  AlsaInputPtr *self; Data_Get_Struct( rbSelf, AlsaInputPtr, self );
  return UINT2NUM( (*self)->rate() );
}

VALUE AlsaInput::wrapChannels( VALUE rbSelf )
{
  AlsaInputPtr *self; Data_Get_Struct( rbSelf, AlsaInputPtr, self );
  return UINT2NUM( (*self)->channels() );
}

VALUE AlsaInput::wrapPrepare( VALUE rbSelf )
{
  try {
    AlsaInputPtr *self; Data_Get_Struct( rbSelf, AlsaInputPtr, self );
    (*self)->prepare();
  } catch ( exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbSelf;
}

