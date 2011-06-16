# hornetseye-alsa - Play audio data using libalsa
# Copyright (C) 2010 Jan Wedekind
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Namespace of Hornetseye computer vision library
module Hornetseye

  # Class for playing sounds using the ALSA library
  #
  # @see http://www.alsa-project.org/
  class AlsaOutput

    class << self

      # Alias for native constructor
      #
      # @return [AlsaOutput] An object for accessing the speakers.
      #
      # @private
      alias_method :orig_new, :new
     
      # Open a sound device for output
      #
      # Open the specified sound device for writing. Note that the desired sample rate
      # may not be supported. In that case the sound library will select a sampling
      # rate near the desired one.
      #
      # @example Open default speakers
      #   require 'hornetseye_alsa'
      #   include Hornetseye
      #   speaker = AlsaOutput.new 'default:0', 44_100, 2
      #
      # @param [String] pcm_name Name of the PCM device
      # @param [Integer] rate Desired sampling rate.
      # @param [Integer] channels Number of channels (1=mono, 2=stereo).
      # @param [Integer] periods Number of audio frames of the output buffer.
      # @param [Integer] frames Size of the audio frames of the output buffer.
      # @return [AlsaOutput] An object for accessing the speakers.
      #
      # @see #rate
      def new( pcm_name = 'default:0', rate = 48000, channels = 2, periods = 8,
               frames = 1024 )
        orig_new pcm_name, rate, channels, periods, frames
      end

    end

    # Alias for native method
    #
    # @private
    alias_method :orig_write, :write

    # Write an audio frame to the sound device
    #
    # The audio data is written to the output buffer of the sound device. Playback is
    # resumed if a buffer underflow occurred earlier. The first dimension of the array
    # with the audio data must match the number of channels of the audio device. The
    # second dimension is the number of audio samples.
    #
    # A blocking write operation is used. I.e. the program is blocked until there is
    # sufficient space in the audio output buffer.
    #
    # @example Play a 400Hz tune for 3 seconds
    #   require 'hornetseye_alsa'
    #   include Hornetseye
    #   speaker = AlsaOutput.new 'default:0', 44_100, 2
    #   L = 44_100 / 400
    #   wave = lazy( 2, L ) { |j,i| Math.sin( i * 2 * Math::PI / L ) * 0x7FFF }.to_sint
    #   ( 3 * 400 ).times { speaker.write wave }
    #
    # @param [Node] frame A two-dimensional array of short-integer audio samples.
    #
    # @return [Node] Returns the parameter +frame+.
    def write( frame )
      if frame.typecode != SINT
        raise "Audio data must be of type SINT (but was #{frame.typecode})"
      end
      if frame.dimension != 2
        raise "Audio frame must have two dimensions (but had #{frame.dimension})"
      end
      if frame.shape.first != channels
        raise "Audio frame must have #{channels} channel(s) but had " +
              "#{frame.shape.first}"
      end
      orig_write Hornetseye::Sequence(UBYTE).new 2 * frame.size, :memory => frame.memory 
      frame
    end

  end

end

