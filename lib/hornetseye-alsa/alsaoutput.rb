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

  class AlsaOutput

    class << self

      alias_method :orig_new, :new

      def new( pcm_name = 'default:0', rate = 48000, channels = 2, periods = 2,
               frames = 1024 )
        orig_new pcm_name, rate, channels, periods, frames
      end

    end

    alias_method :orig_write, :write

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
      orig_write Sequence( UBYTE, 2 * frame.size ).new( frame.memory )
    end

    alias_method :orig_wait, :wait

    def wait( time = 1 )
      orig_wait ( time * 1000 ).to_i
    end

  end

end

