// ColorBrewer.cpp: implementation of the CColorBrewer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ColorBrewer.h"
#include "ColorDef.h"

#include <shlwapi.h>

//////////////////////////////////////////////////////////////////////

BYTE TEXTFRIENDLY_TOLERANCE = (255 / 5); // 20%

//////////////////////////////////////////////////////////////////////

CColorBrewer::CColorBrewer(DWORD dwFlags) : m_dwFlags(dwFlags)
{
}

int CColorBrewer::GetAllPalettes(CColorBrewerPaletteArray& aPalettes) const
{
#ifdef _DEBUG
	ValidatePalettes();
#endif
	aPalettes.RemoveAll();

	for (int nGroup = 0; nGroup < COLORBREWER_NUMGROUPS; nGroup++)
		CopyGroup(COLORBREWER_GROUPS[nGroup], aPalettes);

	return aPalettes.GetSize();
}

int CColorBrewer::GetPalettes(LPCTSTR szName, CColorBrewerPaletteArray& aPalettes, BOOL bPartial, BOOL bAppend) const
{
#ifdef _DEBUG
	ValidatePalettes();
#endif
	if (!bAppend)
		aPalettes.RemoveAll();

	for (int nGroup = 0; nGroup < COLORBREWER_NUMGROUPS; nGroup++)
	{
		const COLORBREWER_PALETTEGROUP& group = COLORBREWER_GROUPS[nGroup];

		if (GroupMatches(group, szName, bPartial))
		{
			for (int nPal = 0; nPal < group.nNumPalettes; nPal++)
				CopyPalette(group.palettes[nPal], aPalettes);
		}
	}

	return aPalettes.GetSize();
}

int CColorBrewer::GetPalettes(int nNumColors, CColorBrewerPaletteArray& aPalettes, BOOL bAppend) const
{
#ifdef _DEBUG
	ValidatePalettes();
#endif
	if (!bAppend)
		aPalettes.RemoveAll();

	if (nNumColors < 3)
	{
		ASSERT(0);
		return 0;
	}

	for (int nGroup = 0; nGroup < COLORBREWER_NUMGROUPS; nGroup++)
	{
		const COLORBREWER_PALETTEGROUP& group = COLORBREWER_GROUPS[nGroup];
		BOOL bMatchFound = FALSE;

		for (int nPal = 0; nPal < group.nNumPalettes; nPal++)
		{
			if (PaletteMatches(group.palettes[nPal], nNumColors))
			{
				CopyPalette(group.palettes[nPal], aPalettes);
				bMatchFound = TRUE;

				break; // one palette per group
			}
		}

		if (!bMatchFound && (m_dwFlags & CBF_SYNTHESIZE))
		{
			CColorBrewerPalette pal;

			if (SynthesizePalette(nNumColors, group, pal))
			{
				aPalettes.Add(pal);
			}
		}
	}

	return aPalettes.GetSize();
}

int CColorBrewer::GetPalettes(COLORBREWER_PALETTETYPE nType, CColorBrewerPaletteArray& aPalettes, int nNumColors, BOOL bAppend) const
{
#ifdef _DEBUG
	ValidatePalettes();
#endif
	if (!bAppend)
		aPalettes.RemoveAll();

	if ((nNumColors != -1) && (nNumColors < 3))
	{
		ASSERT(0);
		return 0;
	}

	for (int nGroup = 0; nGroup < COLORBREWER_NUMGROUPS; nGroup++)
	{
		const COLORBREWER_PALETTEGROUP& group = COLORBREWER_GROUPS[nGroup];
		BOOL bMatchFound = FALSE;

		if (GroupMatches(group, nType))
		{
			for (int nPal = 0; nPal < group.nNumPalettes; nPal++)
			{
				if (PaletteMatches(group.palettes[nPal], nNumColors))
				{
					CopyPalette(group.palettes[nPal], aPalettes);
					bMatchFound = TRUE;

					if (nNumColors != -1)
						break; // one palette per group
				}
			}

			if (!bMatchFound && (nNumColors != -1) && (m_dwFlags & CBF_SYNTHESIZE))
			{
				CColorBrewerPalette pal;

				if (SynthesizePalette(nNumColors, group, pal))
				{
					aPalettes.Add(pal);
				}
			}
		}
	}

	return aPalettes.GetSize();
}

BOOL CColorBrewer::ValidatePalettes()
{
	int nGroup = COLORBREWER_NUMGROUPS;
	
	while (nGroup--)
	{
		if (!ValidateGroup(COLORBREWER_GROUPS[nGroup]))
		{
			ASSERT(0);
			return FALSE;
		}
	}
	
	// all good
	return TRUE;
}

BOOL CColorBrewer::ValidateGroup(const COLORBREWER_PALETTEGROUP& group)
{
	if ((group.nNumPalettes <= 0) || (group.nNumPalettes > COLORBREWER_MAXPALETTES))
	{
		ASSERT(0);
		return FALSE;
	}
	
	// Check trailing sentinel value
	if (group.nNumPalettes < COLORBREWER_MAXPALETTES)
	{
		if (group.palettes[group.nNumPalettes].nNumColors != 0)
		{
			ASSERT(0);
			return FALSE;
		}
	}
	
	// check palettes
	int nPal = group.nNumPalettes;
	
	while (nPal--)
	{
		if (!ValidatePalette(group.palettes[nPal]))
		{
			ASSERT(0);
			return FALSE;
		}
	}
	
	// all good
	return TRUE;
}

BOOL CColorBrewer::ValidatePalette(const COLORBREWER_PALETTE& pal)
{
	if (pal.nNumColors > COLORBREWER_MAXCOLORS)
	{
		ASSERT(0);
		return FALSE;
	}
	
	// Check trailing sentinel value
	if (pal.nNumColors < COLORBREWER_MAXCOLORS)
	{
		if (pal.crPalette[pal.nNumColors] != CLR_NONE)
		{
			ASSERT(0);
			return FALSE;
		}
	}
	
	// Check colors
	int nCol = pal.nNumColors;
	
	while (nCol--)
	{
		if (pal.crPalette[nCol] == CLR_NONE)
		{
			ASSERT(0);
			return FALSE;
		}
	}
	
	// all good
	return TRUE;
}

BOOL CColorBrewer::GroupMatches(const COLORBREWER_PALETTEGROUP& group, LPCTSTR szName, BOOL bPartial)
{
	if (bPartial)
		return (StrStrI(group.szName, szName) != NULL);
	
	// else
	return (StrCmpI(group.szName, szName) != NULL);
}

BOOL CColorBrewer::GroupMatches(const COLORBREWER_PALETTEGROUP& group, COLORBREWER_PALETTETYPE nType)
{
	return (group.nType == nType);
}

BOOL CColorBrewer::GroupMatches(const COLORBREWER_PALETTEGROUP& group, int nNumColors) const
{
	if (nNumColors == -1)
		return TRUE;
	
	int nPal = group.nNumPalettes;
	
	while (nPal--)
	{
		if (PaletteMatches(group.palettes[nPal], nNumColors))
			return TRUE;
	}
	
	return FALSE;
}

BOOL CColorBrewer::PaletteMatches(const COLORBREWER_PALETTE& pal, int nNumColors) const
{
	if (nNumColors == -1)
		return TRUE;
	
	if (m_dwFlags & CBF_TEXTSAFE)
		return (GetTextSafeColorCount(pal) == nNumColors);

	// else
	return (pal.nNumColors == nNumColors);
}

BOOL CColorBrewer::SynthesizePalette(int nNumColors, const COLORBREWER_PALETTEGROUP& groupFrom, CColorBrewerPalette& palTo) const
{
	// Sanity checks
	if (!(m_dwFlags & CBF_SYNTHESIZE))
	{
		ASSERT(0);
		return FALSE;
	}

	// Can only synthesize from a group not having a
	// native palette with that number of colours
	if (GroupMatches(groupFrom, nNumColors))
	{
		ASSERT(0);
		return FALSE;
	}

	int nPal = groupFrom.nNumPalettes;

	while (nPal--)
	{
		if (SynthesizePalette(nNumColors, groupFrom.nType, groupFrom.palettes[nPal], palTo))
			return TRUE;
	}
	
	return FALSE;
}

BOOL CColorBrewer::SynthesizePalette(int nNumColors, COLORBREWER_PALETTETYPE nTypeFrom, const COLORBREWER_PALETTE& palFrom, CColorBrewerPalette& palTo) const
{
	switch (nTypeFrom)
	{
	case CBPT_SEQUENTIAL:
		{
			if (nNumColors % 2) // odd number
			{
				CColorBrewerPalette temp;
				int nReqColorCount = ((nNumColors / 2) + 1);

				if (CopyPalette(palFrom, temp) != nReqColorCount)
					return FALSE;

				palTo.SetSize(nNumColors);

				for (int nCol = 0; nCol < nNumColors; nCol++)
				{
					int nTempCol = (nCol / 2);

					switch (nCol % 2)
					{
					case 0: // 0, 2, 4, 6, 8, 10, etc
						palTo[nCol] = temp[nTempCol];
						break;

					case 1: // 1, 3, 5, 7, 9, 11, etc
						palTo[nCol] = BlendColors(temp[nTempCol], 1, palTo[nTempCol + 1], 1); // 50/50
						break;
					}
				}
			}
			else // even number
			{
				CColorBrewerPalette temp;
				int nReqColorCount = ((nNumColors / 3) + 1);

				if (CopyPalette(palFrom, temp) != nReqColorCount)
					return FALSE;

				palTo.SetSize(nNumColors);

				for (int nCol = 0; nCol < nNumColors; nCol++)
				{
					int nTempCol = (nCol / 3);

					switch (nCol % 3)
					{
					case 0: // 0, 3, 6, 9, etc
						palTo[nCol] = temp[nTempCol];
						break;

					case 1: // 1, 4, 7, 10, etc
						palTo[nCol] = BlendColors(temp[nTempCol], 2, palTo[nTempCol + 1], 1); // 66/33
						break;

					case 2: // 2, 5, 8, 11, etc
						palTo[nCol] = BlendColors(temp[nTempCol], 1, palTo[nTempCol + 1], 2); // 33/66
						break;
					}
				}
			}
		}
		break;

	case CBPT_DIVERGING:
		{
			CColorBrewerPalette temp;
			int nPalColorCount = CopyPalette(palFrom, temp);

			// TODO
		}
		break;

	case CBPT_QUALITATIVE:
		// Not supported because no colour has any relationship
		// to those colours adjacent to it and therefore blending
		// adjacent colours in any way makes no sense
		return FALSE;

	default:
		ASSERT(0);
		return FALSE;
	}

	return FALSE;
}

int CColorBrewer::FindPalette(const COLORBREWER_PALETTEGROUP& group, int nNumColors) const
{
	for (int nPal = 0; nPal < group.nNumPalettes; nPal++)
	{
		if (PaletteMatches(group.palettes[nPal], nNumColors))
			return nPal;
	}

	// else
	return -1;
}

COLORREF CColorBrewer::BlendColors(COLORREF color1, int nAmount1, COLORREF color2, int nAmount2)
{
	BYTE nRed = (BYTE)(((GetRValue(color1) * nAmount1) + (GetRValue(color2) * nAmount2)) / (nAmount1 + nAmount2));
	BYTE nGreen = (BYTE)(((GetGValue(color1) * nAmount1) + (GetGValue(color2) * nAmount2)) / (nAmount1 + nAmount2));
	BYTE nBlue = (BYTE)(((GetBValue(color1) * nAmount1) + (GetBValue(color2) * nAmount2)) / (nAmount1 + nAmount2));

	return RGB(nRed, nGreen, nBlue);
}

void CColorBrewer::CopyGroup(const COLORBREWER_PALETTEGROUP& group, CColorBrewerPaletteArray& aPalettes) const 
{
	for (int nPal = 0; nPal < group.nNumPalettes; nPal++)
		CopyPalette(group.palettes[nPal], aPalettes);
}

int CColorBrewer::CopyPalette(const COLORBREWER_PALETTE& pal, CColorBrewerPaletteArray& aPalettes) const
{
	int nNew = aPalettes.Add(CColorBrewerPalette());

	return CopyPalette(pal, aPalettes[nNew]);
}

int CColorBrewer::CopyPalette(const COLORBREWER_PALETTE& palFrom, CColorBrewerPalette& aTo) const
{
	int nCol = palFrom.nNumColors;

	if (m_dwFlags & CBF_TEXTSAFE)
	{
		while (nCol--)
		{
			if (IsTextSafeColor(palFrom.crPalette[nCol]))
				aTo.Add(palFrom.crPalette[nCol]);
		}
	}
	else // all
	{
		ASSERT(sizeof(COLORREF) == sizeof(DWORD));

		aTo.SetSize(nCol);
		CopyMemory(aTo.GetData(), palFrom.crPalette, (nCol * sizeof(COLORREF)));
	}

	return aTo.GetSize();
}

BOOL CColorBrewer::IsTextSafeColor(COLORREF color)
{
	BYTE nBkgndLum = RGBX(GetSysColor(COLOR_WINDOW)).Luminance();
	BYTE nColorLum = RGBX(color).Luminance();

	return (abs(nBkgndLum - nColorLum) >= TEXTFRIENDLY_TOLERANCE);
}

int CColorBrewer::GetTextSafeColorCount(const COLORBREWER_PALETTE& palFrom)
{
	int nCol = palFrom.nNumColors, nCount = 0;

	while (nCol--)
	{
		if (IsTextSafeColor(palFrom.crPalette[nCol]))
			nCount++;
	}

	return nCount;
}