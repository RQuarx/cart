#include "cell.hh"

using cart::cell_attribute;
using cart::cell_color;


void cell_attribute::set(flag f, bool state) noexcept { state ? (bits |= f) : (bits &= ~f); }
auto cell_attribute::has(flag f) const noexcept -> bool { return (bits & f) != 0; }
void cell_attribute::apply_from_other(const cell_attribute &other) noexcept { bits = other.bits; }


void cell_color::set_background(color color) noexcept { this->bg = color; }
void cell_color::set_foreground(color color) noexcept { this->fg = color; }
auto cell_color::get_background() const noexcept -> color { return this->bg; }
auto cell_color::get_foreground() const noexcept -> color { return this->fg; }
auto cell_color::inverse() const noexcept -> cell_color
{
    cell_color c = *this;
    std::swap(c.bg, c.fg);
    return c;
}


void cell_color::apply_from_other(const cell_color &other) noexcept
{
    this->set_background(other.get_background());
    this->set_foreground(other.get_foreground());
}
