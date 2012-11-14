/* HornetsEye - Computer Vision with Ruby
   Copyright (C) 2012  Jan Wedekind

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

AlsaOutput::AlsaOutput(const string &pcmName, unsigned int rate,
                       unsigned int channels) throw (Error):
  m_pcmHandle(NULL), m_pcmName(pcmName), m_rate(rate), m_channels(channels),
  m_periodSize(1024), m_threadInitialised(false), m_start(0), m_count(0),
  m_size(rate)
{
  try {
    snd_pcm_hw_params_t *hwParams;
    snd_pcm_hw_params_alloca(&hwParams);
    int err = snd_pcm_open(&m_pcmHandle, m_pcmName.c_str(), SND_PCM_STREAM_PLAYBACK,
                           SND_PCM_NONBLOCK);
    ERRORMACRO(err >= 0, Error, , "Error opening PCM device \"" << m_pcmName
               << "\": " << snd_strerror(err));
    err = snd_pcm_hw_params_any(m_pcmHandle, hwParams);
    ERRORMACRO(err >= 0, Error, , "Unable to configure the PCM device \""
               << m_pcmName << "\": " << snd_strerror(err));
    err = snd_pcm_hw_params_set_access(m_pcmHandle, hwParams,
                                       SND_PCM_ACCESS_RW_INTERLEAVED);
    ERRORMACRO(err >= 0, Error, , "Error setting PCM device \""
               << m_pcmName << "\" to interlaced access: " << snd_strerror(err));
    err = snd_pcm_hw_params_set_format(m_pcmHandle, hwParams, SND_PCM_FORMAT_S16_LE);
    ERRORMACRO(err >= 0, Error, , "Error setting PCM device \"" << m_pcmName
               << "\" to 16-bit signed integer format: " << snd_strerror(err));
    err = snd_pcm_hw_params_set_rate_near( m_pcmHandle, hwParams, &m_rate, 0 );
    ERRORMACRO(err >= 0, Error, , "Error setting sampling rate of PCM device \""
               << m_pcmName << "\" to " << rate << " Hz: " << snd_strerror(err));
    err = snd_pcm_hw_params_set_channels(m_pcmHandle, hwParams, channels);
    ERRORMACRO(err >= 0, Error, , "Error setting number of channels of PCM device \""
               << m_pcmName << "\" to " << channels << ": " << snd_strerror(err));
    unsigned int bufferTime = 500000;
    err = snd_pcm_hw_params_set_buffer_time_near(m_pcmHandle, hwParams, &bufferTime, NULL);
    ERRORMACRO(err >= 0, Error, , "Error setting buffer time of PCM device \""
               << m_pcmName << "\" to " << bufferTime << " us: " << snd_strerror(err));
    unsigned int periods = 16;
    err = snd_pcm_hw_params_set_periods_near(m_pcmHandle, hwParams, &periods, NULL);
    ERRORMACRO(err >= 0, Error, , "Error setting periods of PCM device \""
               << m_pcmName << "\" to " << periods << ": " << snd_strerror(err));
    err = snd_pcm_hw_params( m_pcmHandle, hwParams );
    ERRORMACRO( err >= 0, Error, , "Error setting parameters of PCM device \""
                << m_pcmName << "\": " << snd_strerror( err ) );
    err = snd_pcm_hw_params_get_period_size(hwParams, &m_periodSize, NULL);
    ERRORMACRO( err >= 0, Error, , "Error getting period size of PCM device \""
                << m_pcmName << "\": " << snd_strerror( err ) );
    err = pthread_mutex_init(&m_mutex, NULL);
    ERRORMACRO(err == 0, Error, , "Error initialising mutex: " << strerror(err));
  } catch (Error &e) {
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
    drain();
    pthread_mutex_destroy(&m_mutex);
    snd_pcm_close( m_pcmHandle );
    m_pcmHandle = NULL;
  };
}

void AlsaOutput::write(SequencePtr frame) throw (Error)
{
  ERRORMACRO(m_pcmHandle != NULL, Error, , "PCM device \"" << m_pcmName
             << "\" is not open. Did you call \"close\" before?");
  int n = frame->size() / (2 * m_channels);
  lock();
  if (!m_data.get()) {
    if (m_threadInitialised) pthread_join(m_thread, NULL);
    while(m_size < n) m_size = 2 * m_size;
    m_data = boost::shared_array<short int>(new short int[m_size * m_channels]);
    m_start = 0;
    m_count = 0;
    pthread_create(&m_thread, NULL, staticThreadFunc, this);
  };
  if (m_count + n > m_size) {
    int m_size_new = m_size;
    while(m_size_new < m_count + n) m_size_new = 2 * m_size_new;
    boost::shared_array<short int> data(new short int[m_size_new * m_channels]);
    if (m_start + m_count > m_size) {
      memcpy(data.get(), m_data.get() + m_start * m_channels, (m_size - m_start) * 2 * m_channels);
      memcpy(data.get() + (m_size - m_start) * m_channels, m_data.get(), (m_start + m_count - m_size) * 2 * m_channels);
    } else
      memcpy(data.get(), m_data.get() + m_start * m_channels, m_count * 2 * m_channels);
    m_data = data;
    m_start = 0;
    m_size = m_size_new;
  };
  int offset = m_start + m_count;
  if (offset > m_size) offset -= m_size;
  if (offset + n > m_size) {
    memcpy(m_data.get() + offset * m_channels, frame->data(), (m_size - offset) * 2 * m_channels);
    memcpy(m_data.get(), frame->data() + (m_size - offset) * m_channels, (n + m_size - offset) * 2 * m_channels);
  } else
    memcpy(m_data.get() + offset * m_channels, frame->data(), n * 2 * m_channels);
  m_count += n;
  unlock();
}

void AlsaOutput::drop(void) throw (Error)
{
  ERRORMACRO( m_pcmHandle != NULL, Error, , "PCM device \"" << m_pcmName
              << "\" is not open. Did you call \"close\" before?" );
  lock();
  m_data.reset();
  m_count = 0;
  unlock();
  snd_pcm_drop(m_pcmHandle);
}

void AlsaOutput::drain(void) throw (Error)
{
  if (m_threadInitialised) {
    pthread_join(m_thread, NULL);
    m_threadInitialised = false;
  }
  ERRORMACRO(m_pcmHandle != NULL, Error, , "PCM device \"" << m_pcmName
             << "\" is not open. Did you call \"close\" before?");
  snd_pcm_drain(m_pcmHandle);
}

unsigned int AlsaOutput::rate(void)
{
  return m_rate;
}

unsigned int AlsaOutput::channels(void)
{
  return m_channels;
}

int AlsaOutput::avail(void) throw (Error)
{
  snd_pcm_sframes_t frames;
  int err = 0;
  while ( ( frames = snd_pcm_avail( m_pcmHandle ) ) < 0 ) {
    err = snd_pcm_recover( m_pcmHandle, frames, 1 );
    ERRORMACRO( err >= 0, Error, , "Error querying number of available frames for "
              "update of PCM device \"" << m_pcmName << "\": "
              << snd_strerror( err ) );
  };
  return frames;
}

int AlsaOutput::delay(void) throw (Error)
{
  ERRORMACRO(m_pcmHandle != NULL, Error, , "PCM device \"" << m_pcmName
             << "\" is not open. Did you call \"close\" before?");
  lock();
  snd_pcm_sframes_t frames;
  int err;
  while ((err = snd_pcm_delay(m_pcmHandle, &frames)) < 0) {
    err = snd_pcm_recover(m_pcmHandle, err, 1);
    if (err < 0) {
      unlock();
      ERRORMACRO(false, Error, , "Error querying number of available frames for "
                 "update of PCM device \"" << m_pcmName << "\": "
                 << snd_strerror(err));
    };
  };
  frames += m_count;
  unlock();
  return frames;
}

void AlsaOutput::lock(void)
{
  pthread_mutex_lock( &m_mutex );
}

void AlsaOutput::unlock(void)
{
  pthread_mutex_unlock( &m_mutex );
}

void AlsaOutput::writei(short int *data, int count) throw (Error)
{
  int err;
  while ((err = snd_pcm_writei(m_pcmHandle, data, count)) < 0) {
    if (err == -EBADFD)
      err = snd_pcm_prepare(m_pcmHandle);
    else
      err = snd_pcm_recover(m_pcmHandle, err, 1);
    ERRORMACRO(err >= 0, Error, , "Error writing audio frames to PCM device \""
               << m_pcmName << "\": " << snd_strerror(err));
  };
  ERRORMACRO(count == err, Error, , "Only managed to write " << err << " of " << count
             << " frames to PCM device \"" << m_pcmName << "\"");
}

void AlsaOutput::threadFunc(void)
{
  bool quit = false;
  while (!quit) {
    snd_pcm_wait(m_pcmHandle, 1000);
    try {
      lock();
      int n = m_periodSize;
      if (n > m_count) n = m_count;
      if (m_count <= 0) m_data.reset();
      if (m_start >= m_size) m_start -= m_size;
      if (m_data.get()) {
        if (m_start + n > m_size) {
          writei(m_data.get() + m_start * m_channels, m_size - m_start);
          writei(m_data.get(), m_start + n - m_size);
        } else
          writei(m_data.get() + m_start * m_channels, n);
        m_start += n;
        m_count -= n;
      } else
        quit = true;
      unlock();
    } catch (Error &e) {
      quit = true;
      unlock();
    }
  };
}

void *AlsaOutput::staticThreadFunc( void *self )
{
  ((AlsaOutput *)self)->threadFunc();
  return self;
}

VALUE AlsaOutput::registerRubyClass( VALUE rbModule )
{
  cRubyClass = rb_define_class_under( rbModule, "AlsaOutput", rb_cObject );
  rb_define_singleton_method(cRubyClass, "new", RUBY_METHOD_FUNC(wrapNew), 3);
  rb_define_method( cRubyClass, "close", RUBY_METHOD_FUNC( wrapClose ), 0 );
  rb_define_method( cRubyClass, "write", RUBY_METHOD_FUNC( wrapWrite ), 1 );
  rb_define_method( cRubyClass, "drop", RUBY_METHOD_FUNC( wrapDrop ), 0 );
  rb_define_method( cRubyClass, "drain", RUBY_METHOD_FUNC( wrapDrain ), 0 );
  rb_define_method( cRubyClass, "rate", RUBY_METHOD_FUNC( wrapRate ), 0 );
  rb_define_method( cRubyClass, "channels", RUBY_METHOD_FUNC( wrapChannels ), 0 );
  rb_define_method( cRubyClass, "delay", RUBY_METHOD_FUNC( wrapDelay ), 0 );
}

void AlsaOutput::deleteRubyObject( void *ptr )
{
  delete (AlsaOutputPtr *)ptr;
}

VALUE AlsaOutput::wrapNew(VALUE rbClass, VALUE rbPCMName, VALUE rbRate, VALUE rbChannels)
{
  VALUE retVal = Qnil;
  try {
    rb_check_type( rbPCMName, T_STRING );
    AlsaOutputPtr ptr(new AlsaOutput(StringValuePtr(rbPCMName),
                                     NUM2UINT(rbRate), NUM2UINT(rbChannels)));
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

VALUE AlsaOutput::wrapDrop( VALUE rbSelf )
{
  try {
    AlsaOutputPtr *self; Data_Get_Struct( rbSelf, AlsaOutputPtr, self );
    (*self)->drop();
  } catch ( exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbSelf;
}

VALUE AlsaOutput::wrapDrain( VALUE rbSelf )
{
  try {
    AlsaOutputPtr *self; Data_Get_Struct( rbSelf, AlsaOutputPtr, self );
    (*self)->drain();
  } catch ( exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbSelf;
}

VALUE AlsaOutput::wrapRate( VALUE rbSelf )
{
  AlsaOutputPtr *self; Data_Get_Struct( rbSelf, AlsaOutputPtr, self );
  return UINT2NUM( (*self)->rate() );
}

VALUE AlsaOutput::wrapChannels( VALUE rbSelf )
{
  AlsaOutputPtr *self; Data_Get_Struct( rbSelf, AlsaOutputPtr, self );
  return UINT2NUM( (*self)->channels() );
}

VALUE AlsaOutput::wrapDelay( VALUE rbSelf )
{
  VALUE rbRetVal = Qnil;
  try {
    AlsaOutputPtr *self; Data_Get_Struct( rbSelf, AlsaOutputPtr, self );
    rbRetVal = INT2NUM( (*self)->delay() );
  } catch ( exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return rbRetVal;
}
