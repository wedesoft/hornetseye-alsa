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

  class AlsaInput

    attr_reader :rate

    attr_reader :channels

    def close
    end

    def avail
    end

    def delay
    end

    def prepare
    end

  end
  
  class AlsaOutput

    # Get the sampling rate of the sound device
    #
    # The sampling rate may be different to the desired sampling rate specified in
    # the constructor.
    #
    # @return [Integer] The sampling rate of the sound device.
    attr_reader :rate

    # Number of audio channels
    #
    # @return [Integer] Number of audio channels (1=mono, 2=stereo).
    attr_reader :channels

    # Close the audio device
    def close
    end

    # Drop content of audio output buffer
    def drop
    end

    # Wait until audio buffer underflows
    def drain
    end

    # Space available for writing in the audio buffer
    #
    # @return [Integer] Number of audio samples which can be written to the audio
    #         buffer.
    def avail
    end

    # Number of audio samples in the audio buffer
    #
    # Returns the number of audio samples left in the audio buffer. This can be used
    # to properly synchronise video display with audio output.
    #
    # @return [Integer] Number of audio samples left to play.
    def delay
    end

    # Reset the sound device.
    #
    # One needs to call this method if one wants to resume playing audio samples after
    # calling #drop or #drain.
    def prepare
    end

  end

end

