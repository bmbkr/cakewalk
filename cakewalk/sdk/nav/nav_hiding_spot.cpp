#include "nav_hiding_spot.h"

namespace nav_mesh {
	nav_hiding_spot::nav_hiding_spot( nav_buffer& buffer ) {
		load( buffer );
	}

	void nav_hiding_spot::load( nav_buffer& buffer ) {
		m_id = buffer.read< std::uint32_t >( );
		const float vPos[3] = { };
		buffer.read( (void*)&vPos[0], sizeof vPos );
		m_pos = Vector(vPos[0], vPos[1], vPos[2]);
		m_flags = buffer.read< std::uint8_t >( );
	}
}