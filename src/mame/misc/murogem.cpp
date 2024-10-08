// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/*

note:
is it working properly?
no idea how the colours work
is there actually a sound chip?

it executes some illegal opcodes??

M6808: illegal opcode: address FDC2, op 53
M6808: illegal opcode: address FDC5, op 41
M6808: illegal opcode: address FDC6, op 49
M6808: illegal opcode: address FDCB, op 46
M6808: illegal opcode: address FDCE, op 53
M6808: illegal opcode: address FDD1, op 46
M6808: illegal opcode: address FDD3, op 4C
M6808: illegal opcode: address FDC2, op 53
M6808: illegal opcode: address FDC5, op 41
M6808: illegal opcode: address FDC6, op 49
M6808: illegal opcode: address FDCB, op 46
M6808: illegal opcode: address FDCE, op 53
M6808: illegal opcode: address FDD1, op 46
M6808: illegal opcode: address FDD3, op 4C
M6808: illegal opcode: address FDC2, op 53
M6808: illegal opcode: address FDC5, op 41
M6808: illegal opcode: address FDC6, op 49
M6808: illegal opcode: address FDCB, op 46
M6808: illegal opcode: address FDCE, op 53
M6808: illegal opcode: address FDD1, op 46
M6808: illegal opcode: address FDD3, op 4C


PCB layout
2005-07-26
f205v

?Poker?
----------
|----------------------|
| Fully boxed = socket |
|----------------------|


| separation = solder


------------------------------------------------------------------------
|                                                                      |
|      E          D          C          B          A                   |
|                                                                      |
| 1 | 74ls08   | 74ls175  | 74ls175  | 74s288   | CD4098BE  | D31B3100 |
|                                                                      |
|                                                                      |
| 2 | 74ls161  | OSCLLTR  | 74ls367  | 74ls05                          |
|                                                                      |
|                         |                                            |
| 3 | 74ls74   | 74ls04   | M                   | 74ls365              |C
|                         | C                                          |O
|                         | 6                                          |N
| 4 | 74ls139  | 74ls32   | 8                   | 74ls365              |N
|                         | 4                                          |E
|                         | 5                                          |C
| 5 | 74ls32   | 74ls32   | P        | 74ls175                         |T
|                         |                                            |O
|                                                                      |R
| 6 | rom A2   | 74ls273  | ram2114  | 74ls157                         |
|                                                                      |2
|                                                                      |2
| 7 | 74ls166  | 74ls245  | ram2114  | 74ls157                         |
|                                                                      |
|   ----------                                                         |
| 8 | rom A11| | 74ls245  | ram2114  | 74ls157                         |
|   ----------                                                         |
|   ----------                                                         |
| 9 | rom A10| | 8017DK-S6802P                                         |
|   ----------                                                         |
|                                                                      |
------------------------------------------------------------------------


*** Notes by Roberto Fresca ***

- Added Muroge Monaco (set 2).
- Added "Las Vegas, Nevada" (hack).
- Added the missing 74s288 dump (32x8 PROM located at 1B) from the new set.
- Confirmed the MC6845 at $4000/$4001.
- Corrected screen size according to MC6845 registers.
- Corrected visible area according to MC6845 registers. The last characters line looks odd, but should be visible.
- There are not illegal opcodes. The above mentioned are in fact strings (plain ASCII text).

MC6845 registers:
reg (hex):  00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F  10  11
val (hex):  27  20  22  04  26  00  20  20  00  07  00  00  80  00  00  00  ns  ns

- Added Muroge Monaco (set 3). This game has only 1 coin slot.

- Figured out the sound. It's a Delta-Sigma DAC (driven by bit3 of the output port
  at $7000.

- Fixed the GFX decode of Las Vegas.

- The real game title is unknown. Could be "Muroge Monaco", "Las Vegas, Nevada", but
  I strongly think that the original name is different.

*/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "sound/dac.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class murogem_state : public driver_device
{
public:
	murogem_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram")
	{ }

	void murogem(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<dac_bit_interface> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;

	void outport_w(uint8_t data);
	void murogem_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void murogem_map(address_map &map) ATTR_COLD;
};


void murogem_state::outport_w(uint8_t data)
{
/*
   It's a Delta-Sigma DAC (1-bit/Bitstream)

    - bits -
    7654 3210
    ---- x---   Sound DAC.
*/
	m_dac->write(BIT(data, 3));
}


void murogem_state::murogem_map(address_map &map)
{
	map(0x4000, 0x4000).w("crtc", FUNC(mc6845_device::address_w));
	map(0x4001, 0x4001).w("crtc", FUNC(mc6845_device::register_w));
	map(0x5000, 0x5000).portr("IN0");
	map(0x5800, 0x5800).portr("IN1");
	map(0x7000, 0x7000).w(FUNC(murogem_state::outport_w));    /* output port */
	map(0x8000, 0x87ff).ram().share("videoram");
	map(0xf000, 0xffff).rom();
}


static INPUT_PORTS_START( murogem )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet (Replay)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Clear Selection")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Select Card 5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Select Card 4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Select Card 3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Select Card 2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Select Card 1")

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "Reset" ) // reduces credits to 0 and resets game??
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x20, "Scoring" )
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x20, "Type 2" )
	PORT_DIPSETTING(    0x40, "Type 3" )
	PORT_DIPSETTING(    0x60, "Invalid" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END



static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2),RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_murogem )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END


void murogem_state::murogem_palette(palette_device &palette) const
{
}

uint32_t murogem_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	int count = 0x000;
	for (int yy = 0; yy < 32; yy++)
	{
		for (int xx = 0; xx < 32; xx++)
		{
			int const tileno = m_videoram[count] & 0x3f;
			int const attr = m_videoram[count + 0x400] & 0x0f;

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,tileno,attr,0,0,xx*8,yy*8,0);

			count++;
		}
	}

	return 0;
}


void murogem_state::murogem(machine_config &config)
{
	/* basic machine hardware */
	M6802(config, m_maincpu, 8000000);      /* ? MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &murogem_state::murogem_map);
	m_maincpu->set_vblank_int("screen", FUNC(murogem_state::irq0_line_hold));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size((39+1)*8, (38+1)*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(murogem_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_murogem);
	PALETTE(config, m_palette, FUNC(murogem_state::murogem_palette), 0x100);

	mc6845_device &crtc(MC6845(config, "crtc", 750000)); /* ? MHz */
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.375);
}


ROM_START( murogem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a11-8.8e", 0xf000, 0x0800, CRC(1135345e) SHA1(ae23786a6e2b1b077ce1a183d547af42318ac4d9)  )
	ROM_LOAD( "a10-9.9e", 0xf800, 0x0800, CRC(f96791d9) SHA1(12b85e0f8b20ea9331f8cb2b2cf2a4383bdb8003)  )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "a2.6e", 0x0000, 0x0400, CRC(86e053da) SHA1(b7cdddca273204513c818384860883bf54cf9434)  )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "a3.1b", 0x0000, 0x0020, CRC(abddfb6b) SHA1(ed78b93701b5a3bf2053d2584e9a354fb6cec203) )   /* 74s288 at 1B */

ROM_END

ROM_START( murogema )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "poker.02", 0xf000, 0x0800, CRC(e82a5bed) SHA1(556a041cd90e628b6c26824e58cee679dfd59f50)  )
	ROM_LOAD( "poker.03", 0xf800, 0x0800, CRC(f3fe3f12) SHA1(9c73fbaee20098c734431c6af3168986630dcb67)  )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "poker.01", 0x0000, 0x0400, CRC(164d7443) SHA1(1421a3d32d1296a2544da16b51ade94a58e8ba03)  )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "a3.1b", 0x0000, 0x0020, CRC(abddfb6b) SHA1(ed78b93701b5a3bf2053d2584e9a354fb6cec203) )   /* 74s288 at 1B. Originally named 6336.pkr */

ROM_END

/*  Set 802 from EMMA Italian Dumping Team.
    Slightly different. Only one coin in slot.
*/
ROM_START( murogemb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a1.8e", 0xf000, 0x0400, CRC(5b59417a) SHA1(2a2a92b3f8e703ee723ff47d133214e61af8e87d)  )
	ROM_FILL(          0xf400, 0x0400, 0xff ) /* filling the hole */
	ROM_LOAD( "a0.9e", 0xf800, 0x0800, CRC(14ef74fb) SHA1(09ae8156fc76c132cb456aefc1c07a4136d935b8)  )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "a2.6e", 0x0000, 0x0400, CRC(86e053da) SHA1(b7cdddca273204513c818384860883bf54cf9434)  )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "dm74s288.1b", 0x0000, 0x0020, CRC(abddfb6b) SHA1(ed78b93701b5a3bf2053d2584e9a354fb6cec203) )

ROM_END

ROM_START( lasvegas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pk8.8e", 0xf000, 0x0800, CRC(995bd527) SHA1(af96bd0118511b13a755925e3bf5138be61c09d8)  )
	ROM_LOAD( "pk9.9e", 0xf800, 0x0800, CRC(2ab1556e) SHA1(cd7bd377b6a3f6c0f8df61b0da83e55468d599d6)  )

	ROM_REGION( 0x0400, "gfx1", 0 ) // the second half is filled of 0xff.
	ROM_LOAD( "pk6.6e", 0x0000, 0x0400, CRC(78a3593a) SHA1(96ba470f5b0dd6d490eadd09b4b6894e044c66b4)  )
	ROM_IGNORE(                 0x0400)

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "a3.1b", 0x0000, 0x0020, CRC(abddfb6b) SHA1(ed78b93701b5a3bf2053d2584e9a354fb6cec203) )   /* 74s288 at 1B */
ROM_END

} // anonymous namespace


GAME( 198?, murogem,  0,       murogem, murogem, murogem_state, empty_init, ROT0, "<unknown>", "Muroge Monaco (set 1)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 198?, murogema, murogem, murogem, murogem, murogem_state, empty_init, ROT0, "<unknown>", "Muroge Monaco (set 2)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 198?, murogemb, murogem, murogem, murogem, murogem_state, empty_init, ROT0, "<unknown>", "Muroge Monaco (set 3)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 198?, lasvegas, murogem, murogem, murogem, murogem_state, empty_init, ROT0, "hack",      "Las Vegas, Nevada",     MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
