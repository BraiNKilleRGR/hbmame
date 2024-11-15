// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/*

 IGS025 is some kind of state machine / logic device which the game
 uses for various security checks, and to determine the region of the
 game based on string sequences.

 The IGS025 can _NOT_ be swapped between games, so contains at least
 some game specific configuration / programming even if there is a
 large amount of common behavior between games.

*/

#include "emu.h"
#include "iq_igs025.h"


iq_igs025::iq_igs025(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IQ_IGS025, tag, owner, clock)
	, m_execute_external(*this, DEVICE_SELF, FUNC(iq_igs025::no_callback_setup))
{
}

void iq_igs025::no_callback_setup()
{
	printf("igs025 trigger external callback with no external callback setup\n");
}



void iq_igs025::device_start()
{
	// Reset IGS025 stuff
	m_kb_prot_hold = 0;
	m_kb_prot_hilo = 0;
	m_kb_prot_hilo_select = 0;
	m_kb_cmd = 0;
	m_kb_reg = 0;
	m_kb_ptr = 0;
	m_kb_swap = 0;
	m_olds_bs = 0;
	m_kb_cmd3 = 0;

	m_execute_external.resolve();

	save_item(NAME(m_kb_prot_hold));
	save_item(NAME(m_kb_prot_hilo));
	save_item(NAME(m_kb_prot_hilo_select));
	save_item(NAME(m_kb_cmd));
	save_item(NAME(m_kb_reg));
	save_item(NAME(m_kb_ptr));
	save_item(NAME(m_kb_swap));
	save_item(NAME(m_olds_bs));
	save_item(NAME(m_kb_cmd3));
}

void iq_igs025::device_reset()
{
	// Reset IGS025 stuff
	m_kb_prot_hold = 0;
	m_kb_prot_hilo = 0;
	m_kb_prot_hilo_select = 0;
	m_kb_cmd = 0;
	m_kb_reg = 0;
	m_kb_ptr = 0;
	m_kb_swap = 0;


	m_olds_bs = 0;
	m_kb_cmd3 = 0;

}

/****************************************/
/* WRITE */
/****************************************/

void iq_igs025::killbld_igs025_prot_w(offs_t offset, uint16_t data)
{
	if (offset == 0)
	{
		m_kb_cmd = data;
	}
	else
	{
		switch (m_kb_cmd)
		{
			case 0x00:
				m_kb_reg = data;
			break;

			case 0x01: // drgw3
			{
				if (data == 0x0002) { // Execute command
					//printf("execute\n");
					m_execute_external();
				}
			}
			break;

			case 0x02: // killbld
			{
				if (data == 0x0001) { // Execute command
					//printf("execute\n");
					m_execute_external();
					m_kb_reg++;
				}
			}
			break;

			case 0x03:
				m_kb_swap = data;
			break;

			case 0x04:
		//      m_kb_ptr = data; // Suspect. Not good for drgw3
			break;

			case 0x20:
			case 0x21:
			case 0x22:
			case 0x23:
			case 0x24:
			case 0x25:
			case 0x26:
			case 0x27:
				m_kb_ptr++;
				killbld_protection_calculate_hold(m_kb_cmd & 0x0f, data & 0xff);
			break;

		//  default:
		//      logerror("%s: ASIC25 W CMD %X  VAL %X\n", machine().describe_context(), m_kb_cmd, data);
		}
	}
}

void iq_igs025::olds_w(offs_t offset, uint16_t data)
{
	if (offset == 0)
	{
		m_kb_cmd = data;
	}
	else
	{
		switch (m_kb_cmd)
		{
			case 0x00:
				m_kb_reg = data;
			break;

			case 0x02:
				m_olds_bs = ((data & 0x03) << 6) | ((data & 0x04) << 3) | ((data & 0x08) << 1);
			break;

			case 0x03:
			{
				m_execute_external();

				m_kb_cmd3 = ((data >> 4) + 1) & 0x3;
			}
			break;

			case 0x04:
				m_kb_ptr = data;
			break;

			case 0x20:
			case 0x21:
			case 0x22:
			case 0x23:
			case 0x24:
			case 0x25:
			case 0x26:
			case 0x27:
				m_kb_ptr++;
				killbld_protection_calculate_hold(m_kb_cmd & 0x0f, data & 0xff);
			break;

		//  default:
		//      logerror ("unemulated write mode!\n");
		}
	}
}






void iq_igs025::drgw2_d80000_protection_w(offs_t offset, uint16_t data)
{
	if (offset == 0)
	{
		m_kb_cmd = data;
		return;
	}

	switch (m_kb_cmd)
	{
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
			m_kb_ptr++;
			killbld_protection_calculate_hold(m_kb_cmd & 0x0f, data & 0xff);
		break;

	//  case 0x08: // Used only on init..
	//  case 0x09:
	//  case 0x0a:
	//  case 0x0b:
	//  case 0x0c:
	//  break;

	//  case 0x15: // ????
	//  case 0x17:
	//  case 0xf2:
	//  break;

	//  default:
	//      logerror("%s: warning, writing to igs003_reg %02x = %02x\n", machine().describe_context(), m_kb_cmd, data);
	}
}

/****************************************/
/* READ */
/****************************************/

uint16_t iq_igs025::killbld_igs025_prot_r(offs_t offset)
{
	if (offset)
	{
		switch (m_kb_cmd)
		{
		case 0x00:
			return bitswap<8>((m_kb_swap + 1) & 0x7f, 0, 1, 2, 3, 4, 5, 6, 7); // drgw3

		case 0x01:
			return m_kb_reg & 0x7f;

		case 0x02:
			return m_olds_bs | 0x80;

		case 0x03:
			return m_kb_cmd3;

		case 0x05:
		{
						switch (m_kb_ptr)
						{
						case 1:
							return 0x3f00 | ((m_kb_game_id >> 0) & 0xff);

						case 2:
							return 0x3f00 | ((m_kb_game_id >> 8) & 0xff);

						case 3:
							return 0x3f00 | ((m_kb_game_id >> 16) & 0xff);

						case 4:
							return 0x3f00 | ((m_kb_game_id >> 24) & 0xff);

						default: // >= 5
							return 0x3f00 | bitswap<8>(m_kb_prot_hold, 5, 2, 9, 7, 10, 13, 12, 15);
						}
		}

		case 0x40:
			killbld_protection_calculate_hilo();
			return 0; // Read and then discarded

			//  default:
			//      logerror("%s: ASIC25 R CMD %X\n", machine().describe_context(), m_kb_cmd);

			// drgw2 notes
			//  case 0x13: // Read to $80eeb8
			//  case 0x1f: // Read to $80eeb8
			//  case 0xf4: // Read to $80eeb8
			//  case 0xf6: // Read to $80eeb8
			//  case 0xf8: // Read to $80eeb8
			//      return 0;

			//  default:
			//      logerror("%s: warning, reading with igs003_reg = %02x\n", machine().describe_context(), m_kb_cmd);


		}
	}

	return 0;
}


void iq_igs025::killbld_protection_calculate_hold(int y, int z)
{
	unsigned short old = m_kb_prot_hold;

	m_kb_prot_hold = ((old << 1) | (old >> 15));

	m_kb_prot_hold ^= 0x2bad;
	m_kb_prot_hold ^= BIT(z, y);
	m_kb_prot_hold ^= BIT(old, 7) << 0;
	m_kb_prot_hold ^= BIT(~old, 13) << 4;
	m_kb_prot_hold ^= BIT(old, 3) << 11;

	m_kb_prot_hold ^= (m_kb_prot_hilo & ~0x0408) << 1;
}



void iq_igs025::killbld_protection_calculate_hilo()
{
	uint8_t source;

	m_kb_prot_hilo_select++;

	if (m_kb_prot_hilo_select > 0xeb) {
		m_kb_prot_hilo_select = 0;
	}

	source = m_kb_source_data[m_kb_region][m_kb_prot_hilo_select];

	if (m_kb_prot_hilo_select & 1)
	{
		m_kb_prot_hilo = (m_kb_prot_hilo & 0x00ff) | (source << 8);
	}
	else
	{
		m_kb_prot_hilo = (m_kb_prot_hilo & 0xff00) | (source << 0);
	}
}


DEFINE_DEVICE_TYPE(IQ_IGS025, iq_igs025, "iq_igs025", "IQ_IGS025")
