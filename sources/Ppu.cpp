#include "Bus.h"
#include "Cartridge.h"
#include "Ppu.h"

Ppu::Ppu() :
  m_frameComplete(false),
  m_cycle(0),
  m_scanline(0),
  m_sprScreen(256, 240),
  m_sprPatternTable{{128, 128}, {128, 128}},
  m_nmi(false)
{
  m_palScreen[0x00] = tDX::Pixel(84, 84, 84);
  m_palScreen[0x01] = tDX::Pixel(0, 30, 116);
  m_palScreen[0x02] = tDX::Pixel(8, 16, 144);
  m_palScreen[0x03] = tDX::Pixel(48, 0, 136);
  m_palScreen[0x04] = tDX::Pixel(68, 0, 100);
  m_palScreen[0x05] = tDX::Pixel(92, 0, 48);
  m_palScreen[0x06] = tDX::Pixel(84, 4, 0);
  m_palScreen[0x07] = tDX::Pixel(60, 24, 0);
  m_palScreen[0x08] = tDX::Pixel(32, 42, 0);
  m_palScreen[0x09] = tDX::Pixel(8, 58, 0);
  m_palScreen[0x0A] = tDX::Pixel(0, 64, 0);
  m_palScreen[0x0B] = tDX::Pixel(0, 60, 0);
  m_palScreen[0x0C] = tDX::Pixel(0, 50, 60);
  m_palScreen[0x0D] = tDX::Pixel(0, 0, 0);
  m_palScreen[0x0E] = tDX::Pixel(0, 0, 0);
  m_palScreen[0x0F] = tDX::Pixel(0, 0, 0);

  m_palScreen[0x10] = tDX::Pixel(152, 150, 152);
  m_palScreen[0x11] = tDX::Pixel(8, 76, 196);
  m_palScreen[0x12] = tDX::Pixel(48, 50, 236);
  m_palScreen[0x13] = tDX::Pixel(92, 30, 228);
  m_palScreen[0x14] = tDX::Pixel(136, 20, 176);
  m_palScreen[0x15] = tDX::Pixel(160, 20, 100);
  m_palScreen[0x16] = tDX::Pixel(152, 34, 32);
  m_palScreen[0x17] = tDX::Pixel(120, 60, 0);
  m_palScreen[0x18] = tDX::Pixel(84, 90, 0);
  m_palScreen[0x19] = tDX::Pixel(40, 114, 0);
  m_palScreen[0x1A] = tDX::Pixel(8, 124, 0);
  m_palScreen[0x1B] = tDX::Pixel(0, 118, 40);
  m_palScreen[0x1C] = tDX::Pixel(0, 102, 120);
  m_palScreen[0x1D] = tDX::Pixel(0, 0, 0);
  m_palScreen[0x1E] = tDX::Pixel(0, 0, 0);
  m_palScreen[0x1F] = tDX::Pixel(0, 0, 0);

  m_palScreen[0x20] = tDX::Pixel(236, 238, 236);
  m_palScreen[0x21] = tDX::Pixel(76, 154, 236);
  m_palScreen[0x22] = tDX::Pixel(120, 124, 236);
  m_palScreen[0x23] = tDX::Pixel(176, 98, 236);
  m_palScreen[0x24] = tDX::Pixel(228, 84, 236);
  m_palScreen[0x25] = tDX::Pixel(236, 88, 180);
  m_palScreen[0x26] = tDX::Pixel(236, 106, 100);
  m_palScreen[0x27] = tDX::Pixel(212, 136, 32);
  m_palScreen[0x28] = tDX::Pixel(160, 170, 0);
  m_palScreen[0x29] = tDX::Pixel(116, 196, 0);
  m_palScreen[0x2A] = tDX::Pixel(76, 208, 32);
  m_palScreen[0x2B] = tDX::Pixel(56, 204, 108);
  m_palScreen[0x2C] = tDX::Pixel(56, 180, 204);
  m_palScreen[0x2D] = tDX::Pixel(60, 60, 60);
  m_palScreen[0x2E] = tDX::Pixel(0, 0, 0);
  m_palScreen[0x2F] = tDX::Pixel(0, 0, 0);

  m_palScreen[0x30] = tDX::Pixel(236, 238, 236);
  m_palScreen[0x31] = tDX::Pixel(168, 204, 236);
  m_palScreen[0x32] = tDX::Pixel(188, 188, 236);
  m_palScreen[0x33] = tDX::Pixel(212, 178, 236);
  m_palScreen[0x34] = tDX::Pixel(236, 174, 236);
  m_palScreen[0x35] = tDX::Pixel(236, 174, 212);
  m_palScreen[0x36] = tDX::Pixel(236, 180, 176);
  m_palScreen[0x37] = tDX::Pixel(228, 196, 144);
  m_palScreen[0x38] = tDX::Pixel(204, 210, 120);
  m_palScreen[0x39] = tDX::Pixel(180, 222, 120);
  m_palScreen[0x3A] = tDX::Pixel(168, 226, 144);
  m_palScreen[0x3B] = tDX::Pixel(152, 226, 180);
  m_palScreen[0x3C] = tDX::Pixel(160, 214, 228);
  m_palScreen[0x3D] = tDX::Pixel(160, 162, 160);
  m_palScreen[0x3E] = tDX::Pixel(0, 0, 0);
  m_palScreen[0x3F] = tDX::Pixel(0, 0, 0);
}

void Ppu::Clock()
{
  auto IncrementScrollX = [&]()
  {
    if (PPUMASK.b || PPUMASK.s)
    {
      if (m_vramAddr.coarseX == 31)
      {
        m_vramAddr.coarseX = 0;
        m_vramAddr.nametableX = ~m_vramAddr.nametableX;
      }
      else
      {
        m_vramAddr.coarseX++;
      }
    }
  };

  // ==============================================================================
  // Increment the background tile "pointer" one scanline vertically
  auto IncrementScrollY = [&]()
  {
    // Incrementing vertically is more complicated. The visible nametable
    // is 32x30 tiles, but in memory there is enough room for 32x32 tiles.
    // The bottom two rows of tiles are in fact not tiles at all, they
    // contain the "attribute" information for the entire table. This is
    // information that describes which palettes are used for different 
    // regions of the nametable.

    // In addition, the NES doesnt scroll vertically in chunks of 8 pixels
    // i.e. the height of a tile, it can perform fine scrolling by using
    // the fine_y component of the register. This means an increment in Y
    // first adjusts the fine offset, but may need to adjust the whole
    // row offset, since fine_y is a value 0 to 7, and a row is 8 pixels high

    // Ony if rendering is enabled
    if (PPUMASK.b || PPUMASK.s)
    {
      // If possible, just increment the fine y offset
      if (m_vramAddr.fineY < 7)
      {
        m_vramAddr.fineY++;
      }
      else
      {
        // If we have gone beyond the height of a row, we need to
        // increment the row, potentially wrapping into neighbouring
        // vertical nametables. Dont forget however, the bottom two rows
        // do not contain tile information. The coarse y offset is used
        // to identify which row of the nametable we want, and the fine
        // y offset is the specific "scanline"

        // Reset fine y offset
        m_vramAddr.fineY = 0;

        // Check if we need to swap vertical nametable targets
        if (m_vramAddr.coarseY == 29)
        {
          // We do, so reset coarse y offset
          m_vramAddr.coarseY = 0;
          // And flip the target nametable bit
          m_vramAddr.nametableY = ~m_vramAddr.nametableY;
        }
        else if (m_vramAddr.coarseY == 31)
        {
          // In case the pointer is in the attribute memory, we
          // just wrap around the current nametable
          m_vramAddr.coarseY = 0;
        }
        else
        {
          // None of the above boundary/wrapping conditions apply
          // so just increment the coarse y offset
          m_vramAddr.coarseY++;
        }
      }
    }
  };

  // ==============================================================================
  // Transfer the temporarily stored horizontal nametable access information
  // into the "pointer". Note that fine x scrolling is not part of the "pointer"
  // addressing mechanism
  auto TransferAddressX = [&]()
  {
    // Ony if rendering is enabled
    if (PPUMASK.b || PPUMASK.s)
    {
      m_vramAddr.nametableX = m_tranAddr.nametableX;
      m_vramAddr.coarseX = m_tranAddr.coarseX;
    }
  };

  auto TransferAddressY = [&]()
  {
    // Ony if rendering is enabled
    if (PPUMASK.b || PPUMASK.s)
    {
      m_vramAddr.fineY = m_tranAddr.fineY;
      m_vramAddr.nametableY = m_tranAddr.nametableY;
      m_vramAddr.coarseY = m_tranAddr.coarseY;
    }
  };

  auto LoadBackgroundShifters = [&]()
  {
    bg_shifter_pattern_lo = (bg_shifter_pattern_lo & 0xFF00) | bg_next_tile_lsb;
    bg_shifter_pattern_hi = (bg_shifter_pattern_hi & 0xFF00) | bg_next_tile_msb;

    bg_shifter_attrib_lo = (bg_shifter_attrib_lo & 0xFF00) | ((bg_next_tile_attrib & 0b01) ? 0xFF : 0x00);
    bg_shifter_attrib_hi = (bg_shifter_attrib_hi & 0xFF00) | ((bg_next_tile_attrib & 0b10) ? 0xFF : 0x00);
  };

  auto UpdateShifters = [&]()
  {
    if (PPUMASK.b)
    {
      // Shifting background tile pattern row
      bg_shifter_pattern_lo <<= 1;
      bg_shifter_pattern_hi <<= 1;

      // Shifting palette attributes by 1
      bg_shifter_attrib_lo <<= 1;
      bg_shifter_attrib_hi <<= 1;
    }
  };

  // All but 1 of the secanlines is visible to the user. The pre-render scanline
  // at -1, is used to configure the "shifters" for the first visible scanline, 0.
  if (m_scanline >= -1 && m_scanline < 240)
  {
    if (m_scanline == 0 && m_cycle == 0)
    {
      // "Odd Frame" cycle skip
      m_cycle = 1;
    }

    if (m_scanline == -1 && m_cycle == 1)
    {
      // Effectively start of new frame, so clear vertical blank flag
      PPUSTATUS.V = 0;
    }


    if ((m_cycle >= 2 && m_cycle < 258) || (m_cycle >= 321 && m_cycle < 338))
    {
      UpdateShifters();


      // In these cycles we are collecting and working with visible data
      // The "shifters" have been preloaded by the end of the previous
      // scanline with the data for the start of this scanline. Once we
      // leave the visible region, we go dormant until the shifters are
      // preloaded for the next scanline.

      // Fortunately, for background rendering, we go through a fairly
      // repeatable sequence of events, every 2 clock cycles.
      switch ((m_cycle - 1) % 8)
      {
      case 0:
        // Load the current background tile pattern and attributes into the "shifter"
        LoadBackgroundShifters();

        // Fetch the next background tile ID
        // "(vram_addr.reg & 0x0FFF)" : Mask to 12 bits that are relevant
        // "| 0x2000"                 : Offset into nametable space on PPU address bus
        bg_next_tile_id = ReadByPPU(0x2000 | (m_vramAddr.reg & 0x0FFF));

        break;
      case 2:
        bg_next_tile_attrib = ReadByPPU(0x23C0 | (m_vramAddr.nametableY << 11)
          | (m_vramAddr.nametableX << 10)
          | ((m_vramAddr.coarseY >> 2) << 3)
          | (m_vramAddr.coarseX >> 2));

        if (m_vramAddr.coarseY & 0x02) bg_next_tile_attrib >>= 4;
        if (m_vramAddr.coarseX & 0x02) bg_next_tile_attrib >>= 2;
        bg_next_tile_attrib &= 0x03;
        break;

        // Compared to the last two, the next two are the easy ones... :P

      case 4:
        bg_next_tile_lsb = ReadByPPU((PPUCTRL.B << 12)
          + ((uint16_t)bg_next_tile_id << 4)
          + (m_vramAddr.fineY) + 0);

        break;
      case 6:
        // Fetch the next background tile MSB bit plane from the pattern memory
        // This is the same as above, but has a +8 offset to select the next bit plane
        bg_next_tile_msb = ReadByPPU((PPUCTRL.B << 12)
          + ((uint16_t)bg_next_tile_id << 4)
          + (m_vramAddr.fineY) + 8);
        break;
      case 7:
        // Increment the background tile "pointer" to the next tile horizontally
        // in the nametable memory. Note this may cross nametable boundaries which
        // is a little complex, but essential to implement scrolling
        IncrementScrollX();
        break;
      }
    }

    // End of a visible scanline, so increment downwards...
    if (m_cycle == 256)
    {
      IncrementScrollY();
    }

    //...and reset the x position
    if (m_cycle == 257)
    {
      LoadBackgroundShifters();
      TransferAddressX();
    }

    // Superfluous reads of tile id at end of scanline
    if (m_cycle == 338 || m_cycle == 340)
    {
      bg_next_tile_id = ReadByPPU(0x2000 | (m_vramAddr.reg & 0x0FFF));
    }

    if (m_scanline == -1 && m_cycle >= 280 && m_cycle < 305)
    {
      // End of vertical blank period so reset the Y address ready for rendering
      TransferAddressY();
    }
  }

  if (m_scanline == 240)
  {
    // Post Render Scanline - Do Nothing!
  }

  if (m_scanline >= 241 && m_scanline < 261)
  {
    if (m_scanline == 241 && m_cycle == 1)
    {
      PPUSTATUS.V = 1;

      if (PPUCTRL.V)
        m_nmi = true;
    }
  }



  // Composition - We now have background pixel information for this cycle
  // At this point we are only interested in background

  uint8_t bg_pixel = 0x00;   // The 2-bit pixel to be rendered
  uint8_t bg_palette = 0x00; // The 3-bit index of the palette the pixel indexes

  // We only render backgrounds if the PPU is enabled to do so. Note if 
  // background rendering is disabled, the pixel and palette combine
  // to form 0x00. This will fall through the colour tables to yield
  // the current background colour in effect
  if (PPUMASK.b)
  {
    // Handle Pixel Selection by selecting the relevant bit
    // depending upon fine x scolling. This has the effect of
    // offsetting ALL background rendering by a set number
    // of pixels, permitting smooth scrolling
    uint16_t bit_mux = 0x8000 >> fine_x;

    // Select Plane pixels by extracting from the shifter 
    // at the required location. 
    uint8_t p0_pixel = (bg_shifter_pattern_lo & bit_mux) > 0;
    uint8_t p1_pixel = (bg_shifter_pattern_hi & bit_mux) > 0;

    // Combine to form pixel index
    bg_pixel = (p1_pixel << 1) | p0_pixel;

    // Get palette
    uint8_t bg_pal0 = (bg_shifter_attrib_lo & bit_mux) > 0;
    uint8_t bg_pal1 = (bg_shifter_attrib_hi & bit_mux) > 0;
    bg_palette = (bg_pal1 << 1) | bg_pal0;
  }


  // Now we have a final pixel colour, and a palette for this cycle
  // of the current scanline. Let's at long last, draw that ^&%*er :P

  m_sprScreen.SetPixel(m_cycle - 1, m_scanline, GetColourFromPaletteRam(bg_palette, bg_pixel));

  // Fake some noise for now
  //sprScreen.SetPixel(cycle - 1, scanline, palScreen[(rand() % 2) ? 0x3F : 0x30]);

  // Advance renderer - it never stops, it's relentless
  m_cycle++;
  m_frameComplete = false;

  if (m_cycle >= 341)
  {
    m_cycle = 0;
    m_scanline++;
    if (m_scanline >= 261)
    {
      m_scanline = -1;
      m_frameComplete = true;
    }
  }
}

void Ppu::ConnectCartridge(Cartridge * cartridge)
{
  m_cartridge = cartridge;
}

tDX::Sprite& Ppu::GetPatternTable(uint8_t i, uint8_t palette)
{
  for (uint16_t nTileY = 0; nTileY < 16; nTileY++)
  {
    for (uint16_t nTileX = 0; nTileX < 16; nTileX++)
    {
      uint16_t nOffset = nTileY * 256 + nTileX * 16;

      for (uint16_t row = 0; row < 8; row++)
      {
        uint8_t tile_lsb = ReadByPPU(i * 0x1000 + nOffset + row + 0x0000);
        uint8_t tile_msb = ReadByPPU(i * 0x1000 + nOffset + row + 0x0008);

        for (uint16_t col = 0; col < 8; col++)
        {
          uint8_t pixel = (tile_lsb & 0x01) + (tile_msb & 0x01);

          tile_lsb >>= 1; tile_msb >>= 1;

          m_sprPatternTable[i].SetPixel
          (
            nTileX * 8 + (7 - col),
            nTileY * 8 + row,
            GetColourFromPaletteRam(palette, pixel)
          );
        }
      }
    }
  }

  // Finally return the updated sprite representing the pattern table
  return m_sprPatternTable[i];
}

uint8_t Ppu::ReadByCPU(uint16_t addr, bool rdonly)
{
  uint8_t data = 0x00;

  if (rdonly)
  {
    switch (addr)
    {
    case 0x0000: // Control
      data = PPUCTRL.reg;
      break;
    case 0x0001: // Mask
      data = PPUMASK.reg;
      break;
    case 0x0002: // Status
      data = PPUSTATUS.reg;
      break;
    case 0x0003: // OAM Address
      break;
    case 0x0004: // OAM Data
      break;
    case 0x0005: // Scroll
      break;
    case 0x0006: // PPU Address
      break;
    case 0x0007: // PPU Data
      break;
    }
  }
  else
  {
    switch (addr)
    {
      // Control - Not readable
    case 0x0000: break;

      // Mask - Not Readable
    case 0x0001: break;

      // Status
    case 0x0002:
      data = (PPUSTATUS.reg & 0xE0) | (ppu_data_buffer & 0x1F);

      // Clear the vertical blanking flag
      PPUSTATUS.V = 0;

      // Reset Loopy's Address latch flag
      address_latch = 0;
      break;

      // OAM Address
    case 0x0003: break;

      // OAM Data
    case 0x0004: break;

      // Scroll - Not Readable
    case 0x0005: break;

      // PPU Address - Not Readable
    case 0x0006: break;

      // PPU Data
    case 0x0007:
      data = ppu_data_buffer;
      // then update the buffer for next time
      ppu_data_buffer = ReadByPPU(m_vramAddr.reg);
      // However, if the address was in the palette range, the
      // data is not delayed, so it returns immediately
      if (m_vramAddr.reg >= 0x3F00) data = ppu_data_buffer;
      m_vramAddr.reg += (PPUCTRL.I ? 32 : 1);
      break;
    }
  }

  return data;
}

void Ppu::WriteByCPU(uint16_t addr, uint8_t data)
{
  switch (addr)
  {
  case 0x0000: // Control
    PPUCTRL.reg = data;
    m_tranAddr.nametableX = PPUCTRL.Nx;
    m_tranAddr.nametableY = PPUCTRL.Ny;
    break;
  case 0x0001: // Mask
    PPUMASK.reg = data;
    break;
  case 0x0002: // Status
    break;
  case 0x0003: // OAM Address
    break;
  case 0x0004: // OAM Data
    break;
  case 0x0005: // Scroll
    if (address_latch == 0)
    {
      // First write to scroll register contains X offset in pixel space
      // which we split into coarse and fine x values
      fine_x = data & 0x07;
      m_tranAddr.coarseX = data >> 3;
      address_latch = 1;
    }
    else
    {
      // First write to scroll register contains Y offset in pixel space
      // which we split into coarse and fine Y values
      m_tranAddr.fineY = data & 0x07;
      m_tranAddr.coarseY = data >> 3;
      address_latch = 0;
    }
    break;
  case 0x0006: // PPU Address
    if (address_latch == 0)
    {
      // PPU address bus can be accessed by CPU via the ADDR and DATA
      // registers. The fisrt write to this register latches the high byte
      // of the address, the second is the low byte. Note the writes
      // are stored in the tram register...
      m_tranAddr.reg = (uint16_t)((data & 0x3F) << 8) | (m_tranAddr.reg & 0x00FF);
      address_latch = 1;
    }
    else
    {
      // ...when a whole address has been written, the internal vram address
      // buffer is updated. Writing to the PPU is unwise during rendering
      // as the PPU will maintam the vram address automatically whilst
      // rendering the scanline position.
      m_tranAddr.reg = (m_tranAddr.reg & 0xFF00) | data;
      m_vramAddr = m_tranAddr;
      address_latch = 0;
    }
    break;
  case 0x0007: // PPU Data
    WriteByPPU(m_vramAddr.reg, data);
    // All writes from PPU data automatically increment the nametable
    // address depending upon the mode set in the control register.
    // If set to vertical mode, the increment is 32, so it skips
    // one whole nametable row; in horizontal mode it just increments
    // by 1, moving to the next column
    m_vramAddr.reg += (PPUCTRL.I ? 32 : 1);
    break;
  }
}

uint8_t Ppu::ReadByPPU(uint16_t addr, bool rdonly)
{
  uint8_t data = 0x00;
  addr &= 0x3FFF;

  if (addr >= 0x0000 && addr <= 0x1FFF)
  {
    m_cartridge->ReadByPPU(addr, data);
  }
  else if (addr >= 0x2000 && addr <= 0x3EFF)
  {
    addr &= 0x0FFF;

    if (m_cartridge->GetMirroring() == Cartridge::Mirror::Vertical)
    {
      // Vertical
      if (addr >= 0x0000 && addr <= 0x03FF)
        data = m_tblName[0][addr & 0x03FF];
      if (addr >= 0x0400 && addr <= 0x07FF)
        data = m_tblName[1][addr & 0x03FF];
      if (addr >= 0x0800 && addr <= 0x0BFF)
        data = m_tblName[0][addr & 0x03FF];
      if (addr >= 0x0C00 && addr <= 0x0FFF)
        data = m_tblName[1][addr & 0x03FF];
    }
    else if (m_cartridge->GetMirroring() == Cartridge::Mirror::Horizontal)
    {
      // Horizontal
      if (addr >= 0x0000 && addr <= 0x03FF)
        data = m_tblName[0][addr & 0x03FF];
      if (addr >= 0x0400 && addr <= 0x07FF)
        data = m_tblName[0][addr & 0x03FF];
      if (addr >= 0x0800 && addr <= 0x0BFF)
        data = m_tblName[1][addr & 0x03FF];
      if (addr >= 0x0C00 && addr <= 0x0FFF)
        data = m_tblName[1][addr & 0x03FF];
    }
  }
  else if (addr >= 0x3F00 && addr <= 0x3FFF)
  {
    addr &= 0x001F;
    if (addr == 0x0010) addr = 0x0000;
    if (addr == 0x0014) addr = 0x0004;
    if (addr == 0x0018) addr = 0x0008;
    if (addr == 0x001C) addr = 0x000C;
    data = m_tblPalette[addr] & (PPUMASK.Gr ? 0x30 : 0x3F);
  }

  return data;
}

void Ppu::WriteByPPU(uint16_t addr, uint8_t data)
{
  addr &= 0x3FFF;

  if (addr >= 0x0000 && addr <= 0x1FFF)
  {
    m_cartridge->WriteByPPU(addr, data);
  }
  else if (addr >= 0x2000 && addr <= 0x3EFF)
  {
    addr &= 0x0FFF;
    if (m_cartridge->GetMirroring() == Cartridge::Mirror::Vertical)
    {
      // Vertical
      if (addr >= 0x0000 && addr <= 0x03FF)
        m_tblName[0][addr & 0x03FF] = data;
      if (addr >= 0x0400 && addr <= 0x07FF)
        m_tblName[1][addr & 0x03FF] = data;
      if (addr >= 0x0800 && addr <= 0x0BFF)
        m_tblName[0][addr & 0x03FF] = data;
      if (addr >= 0x0C00 && addr <= 0x0FFF)
        m_tblName[1][addr & 0x03FF] = data;
    }
    else if (m_cartridge->GetMirroring() == Cartridge::Mirror::Horizontal)
    {
      // Horizontal
      if (addr >= 0x0000 && addr <= 0x03FF)
        m_tblName[0][addr & 0x03FF] = data;
      if (addr >= 0x0400 && addr <= 0x07FF)
        m_tblName[0][addr & 0x03FF] = data;
      if (addr >= 0x0800 && addr <= 0x0BFF)
        m_tblName[1][addr & 0x03FF] = data;
      if (addr >= 0x0C00 && addr <= 0x0FFF)
        m_tblName[1][addr & 0x03FF] = data;
    }
  }
  else if (addr >= 0x3F00 && addr <= 0x3FFF)
  {
    addr &= 0x001F;
    if (addr == 0x0010) addr = 0x0000;
    if (addr == 0x0014) addr = 0x0004;
    if (addr == 0x0018) addr = 0x0008;
    if (addr == 0x001C) addr = 0x000C;
    m_tblPalette[addr] = data;
  }
}

tDX::Pixel& Ppu::GetColourFromPaletteRam(uint8_t palette, uint8_t pixel)
{
  return m_palScreen[ReadByPPU(0x3F00 + (palette << 2) + pixel) & 0x3F];
}
