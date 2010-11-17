hornetseye-alsa
===============

**Author**:       Jan Wedekind
**Copyright**:    2010
**License**:      GPL

Synopsis
--------

This Ruby extension provides an interface for playing audio data using ALSA.

Installation
------------

To install the Hornetseye ALSA bindings, use the following command:

    $ sudo gem install hornetseye-alsa

You can install from source as follows:

    $ rake
    $ sudo rake install

Usage
-----

Simply run Interactive Ruby:

    $ irb

You can load and use ALSA as shown below. This example will open microphone and
speakers and then write the input of the microphone to the speakers.

    require 'rubygems'
    require 'hornetseye_alsa'
    include Hornetseye
    microphone = AlsaInput.new 'default:0' # or 'default:1' for USB microphone
    speaker = AlsaOutput.new 'default:0', microphone.rate, microphone.channels
    while true
      frame = microphone.read 1024
      speaker.write frame
    end

