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

  # Class for capturing audio samples from an ALSA device
  #
  # @see http://www.alsa-project.org/
  class AlsaInput

    class << self

      # Alias for native constructor
      #
      # @return [AlsaInput] An object for accessing a microphone.
      #
      # @private
      alias_method :orig_new, :new

      # Open a sound device for input
      #
      # Open the specified sound device for reading. Note that the desired sample rate
      # may not be supported. In that case the sound library will choose a sampling
      # rate near the desired one.
      #
      # @example Open standard microphone device
      #   require 'hornetseye_alsa'
      #   include Hornetseye
      #   microphone = AlsaInput.new 'default:0', 44_100, 2
      #
      # @param [String] pcm_name Name of the PCM device
      # @param [Integer] rate Desired sampling rate.
      # @param [Integer] channels Number of channels (1=mono, 2=stereo).
      # @param [Integer] periods Number of audio frames of the input buffer.
      # @param [Integer] frames Size of the audio frames of the input buffer.
      # @return [AlsaInput] An object for accessing the microphone.
      #
      # @see #rate
      def new( pcm_name = 'default:0', rate = 48000, channels = 2, periods = 8,
               frames = 1024 )
        orig_new pcm_name, rate, channels, periods, frames
      end

    end

    # Alias for native method
    #
    # @return [Node] A two-dimensional array with short-integer audio samples.
    #
    # @private
    alias_method :orig_read, :read

    # Read specified number of samples from the sound device
    #
    # Audio data is read from the input buffer.
    #
    # A blocking read operation is used. I.e. the program is blocked until there is
    # sufficient data available in the audio input buffer.
    #
    # @example Read 3 seconds of audio samples
    #   require 'hornetseye_alsa'
    #   include Hornetseye
    #   microphone = AlsaInput.new 'default:0', 44_100, 2
    #   data = microphone.read 3 * 44_100
    #
    # @param [Integer] samples Number of samples to read.
    # @return [Node] A two-dimensional array with short-integer audio samples.
    def read(samples)
      MultiArray.import SINT, orig_read(samples).memory, channels, samples
    end

  end

end

