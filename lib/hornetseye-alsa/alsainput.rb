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

    class << self

      alias_method :orig_new, :new

      def new( pcm_name = 'default:0', rate = 48000, channels = 2, periods = 16,
               frames = 1024 )
        orig_new pcm_name, rate, channels, periods, frames
      end

    end

    alias_method :orig_read, :read

    def read( samples )
      Hornetseye::MultiArray( SINT, channels, samples ).
        new orig_read( samples ).memory
    end

  end

end

