/*
 *   Common font rendering interface, independent of a specific rasterizer
 *
 */

#pragma once

#include "game/coords.h"
#include "game/enum_array.h"
#include "game/graphics.h"
#include <optional>
#include <string_view>

using TEXTRENDER_RECT_ID = unsigned int;

// Concept for a text render session on a single rectangle, abstracting away
// the rasterizer.
template <class T, class FontID> concept TEXTRENDER_SESSION = (
	ENUMARRAY_ID<FontID> && requires (
		T t,
		PIXEL_POINT topleft_rel,
		std::string_view str,
		RGBA color,
		FontID font
	) {
		t.SetFont(font);
		t.SetColor(color);

		// Text display with the current color and font.
		t.Put(topleft_rel, str);

		// Convenience overload to change the color before rendering the text.
		// (Not adding one for the font, since the ID is templated. This
		// allows Put() to be fully implemented within a type-erased base
		// class.)
		t.Put(topleft_rel, str, color);
	}
);

// Concept that describes valid text rendering session functors in game code.
template <typename F, class Session, class FontID>
concept TEXTRENDER_SESSION_FUNC = (
	TEXTRENDER_SESSION<Session, FontID> &&
	requires(F f, Session& s) {
		{ f(s) };
	}
);

// Concept for a text rendering backend.
template <class T, class Session, class FontID> concept TEXTRENDER = requires(
	T t,
	PIXEL_SIZE size,
	WINDOW_POINT dst,
	TEXTRENDER_RECT_ID rect_id,
	TEXTRENDER_SESSION_FUNC<Session, FontID> auto func,
	std::optional<PIXEL_LTWH> subrect
) {
	// Rectangle management
	// --------------------

	// Registers a new text rectangle of the given size, and returns a unique
	// ID to it on success. This ID is valid until the next Clear().
	// This can resize the text surface on the next (pre-)rendering call if the
	// current one can't fit the new rectangle, which will clear any previously
	// rendered text rectangles. Therefore, it should only be called during
	// game state initialization.
	{ t.Register(size) } -> std::same_as<TEXTRENDER_RECT_ID>;

	// Invalidates all registered text rectangles.
	t.Clear();
	// --------------------

	// Retained interface
	// ------------------
	// Explicit prerendering with later blitting.

	{ t.Prerender(rect_id, func) } -> std::same_as<bool>;
	t.Blit(dst, rect_id, subrect);
	// ------------------
};
