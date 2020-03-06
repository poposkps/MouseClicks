#include <iostream>
#include <set>
#include <assert.h>
#include <memory>
#include <vector>
#include <map>
#include <fstream>
#ifndef ONLINE_JUDGE
#include <boost/algorithm/string/trim.hpp>
#include <sstream>
#endif
#include <iomanip>
#include <cassert>

struct Position
{
    Position() : x_(0), y_(0) {}
    Position(int x, int y) : x_(x), y_(y) {}
    bool operator == (const Position & rhs) const { return x_ == rhs.x_&&y_ == rhs.y_; }
    int x_;
    int y_;
};

struct Rect
{
    Rect() {}
    Rect(Position top_left, Position right_bottom) : top_left_(top_left), right_bottom_(right_bottom) {}
    Position top_left_;
    Position right_bottom_;
};

enum class ItemType {ICON, REGIN};

class Item
{
public:
    typedef std::shared_ptr<Item> shared_ptr_t;
    typedef std::shared_ptr<Item const> const_shared_ptr_t;

public:
    Item(const std::string & name) : name_(name) {}
    virtual ~Item() {}

public:
    std::string name() const { return name_; }

public:
    virtual ItemType type() const = 0;
    virtual bool contains(const Position & pos) const = 0;
    virtual bool contains(const Rect & rect) const = 0;
    virtual bool can_select_nearby() const = 0;
    virtual double distence(const Position & pos) const = 0;
    virtual bool is_covered_by(const_shared_ptr_t other) const = 0;

private:
    std::string name_;
};

bool operator < (Item::const_shared_ptr_t lhs, Item::const_shared_ptr_t rhs)
{
    return lhs->name() < rhs->name();
}

class Icon
    : public Item
{
public:
    Icon(const std::string & name, const Position & pos)
        : Item(name), pos_(pos) {}

public:
    virtual ItemType type() const { return ItemType::ICON; }

    virtual bool contains(const Position & pos) const override
    {
        return pos_ == pos;
    }

    virtual bool contains(const Rect & rect) const
    {
        return rect.right_bottom_ == pos_ && rect.top_left_ == pos_;
    }

    virtual bool can_select_nearby() const override
    {
        return true;
    }

    virtual double distence(const Position & pos) const override
    {
        return sqrt(pow(pos_.x_ - pos.x_, 2) + pow(pos_.y_ - pos.y_, 2));
    }

    virtual bool is_covered_by(const_shared_ptr_t other) const override
    {
        return other->contains(pos_);
    }

private:
    Position pos_;
};

class Regin
    : public Item
{
public:
    Regin(const std::string & name, const Rect & rect)
        : Item(name)
        , rect_(rect)
    {

    }

public:
    virtual ItemType type() const { return ItemType::REGIN; }

    virtual bool contains(const Position & pos) const override
    {
        return (pos.x_ >= rect_.top_left_.x_ && pos.x_ <= rect_.right_bottom_.x_)
            && (pos.y_ >= rect_.top_left_.y_ && pos.y_ <= rect_.right_bottom_.y_);
    }

    virtual bool contains(const Rect & rect) const
    {
        return (rect.top_left_.x_ >= rect_.top_left_.x_ && rect.right_bottom_.x_ <= rect_.right_bottom_.x_)
            && (rect.top_left_.y_ >= rect_.top_left_.y_ && rect.right_bottom_.y_ <= rect_.right_bottom_.y_);
    }

    virtual bool can_select_nearby() const override
    {
        return false;
    }

    virtual double distence(const Position & pos) const override
    {
        assert(false);
        return 0;
    }

    virtual bool is_covered_by(const_shared_ptr_t other) const override
    {
        return other->contains(rect_);
    }

private:
    Rect rect_;
};

void parse_input(std::istream & in, std::vector<Item::const_shared_ptr_t> & items, std::vector<Position> & click_points)
{
    int icon_count(0);
    int regin_count(0);

    while (true)
    {
        char type;
        in >> type;
        switch (type)
        {
        case 'I':
            {
                Position pos;
                in >> pos.x_ >> pos.y_;

                icon_count++;
                std::string name = std::to_string(icon_count);
                items.push_back(std::make_shared<Icon>(name, pos));
            }
            break;
        case 'R':
            {
                Position top_left_;
                Position right_bottom_;
                in >> top_left_.x_ >> top_left_.y_ >> right_bottom_.x_ >> right_bottom_.y_;

                std::string name = "A";
                name[0] += regin_count;
                regin_count++;

                items.push_back(std::make_shared<Regin>(name, Rect(top_left_, right_bottom_)));
            }
            break;
        case 'M':
            {
                Position pos;
                in >> pos.x_ >> pos.y_;
                click_points.push_back(pos);
            }
            break;
        case '#':
            {
                return;
            }
        default:
            break;
        }
    }
}

void print_item(Item::const_shared_ptr_t item, std::ostream & out)
{
    switch (item->type())
    {
    case ItemType::ICON:
        out << std::setw(3) << std::right << item->name();
        break;
    case ItemType::REGIN:
        out << item->name();
        break;
    default:
        assert(0);
    }
}

bool is_nth_item_visible(const std::vector<Item::const_shared_ptr_t> & items, size_t nth)
{
    auto checked_item = items[nth];
    for (size_t i = nth + 1 ; i < items.size() ; ++i)
    {
        if (checked_item->is_covered_by(items[i]))
        {
            return false;
        }
    }
    return true;
}

Item::const_shared_ptr_t get_selected_item_by_click_directly(const std::vector<Item::const_shared_ptr_t> & items, const Position & click_pos)
{
    for (auto i = items.rbegin() ; i != items.rend() ; ++i)
    {
        auto item = *i;
        if (item->contains(click_pos))
            return item;
    }
    return Item::const_shared_ptr_t();
}

std::set<Item::const_shared_ptr_t> get_selected_closest_items_by_click_on_blank_area(const std::vector<Item::const_shared_ptr_t> & items, const Position & click_pos)
{
    std::map<double, std::set<Item::const_shared_ptr_t>> distance_to_items_mapping;
    for (size_t i = 0 ; i < items.size() ; ++i)
    {
        auto item = items[i];
        if (item->can_select_nearby() && is_nth_item_visible(items, i))
        {
            distance_to_items_mapping[item->distence(click_pos)].insert(item);
        }
    }
    return distance_to_items_mapping.begin()->second;
}

std::set<Item::const_shared_ptr_t> get_selected_item(const std::vector<Item::const_shared_ptr_t> & items, const Position & click_pos)
{
    Item::const_shared_ptr_t clicked_item = get_selected_item_by_click_directly(items, click_pos);
    if (clicked_item)
    {
        std::set<Item::const_shared_ptr_t> result;
        result.insert(clicked_item);
        return result;
    }

    return get_selected_closest_items_by_click_on_blank_area(items, click_pos);
}

void run(std::istream & in, std::ostream & out)
{
    std::vector<Item::const_shared_ptr_t> items;
    std::vector<Position> click_points;
    parse_input(in, items, click_points);

    for (auto click_pos : click_points)
    {
        auto selected_items = get_selected_item(items, click_pos);
        for (auto selected_item : selected_items)
        {
            print_item(selected_item, out);
        }
        out << std::endl;
    }
}

#ifdef ONLINE_JUDGE

int main(int argc, char** argv)
{
    run(std::cin, std::cout);
    return 0;
}

#else

int main(int argc, char** argv)
{
    std::ifstream stream_in("input.txt");
    std::stringstream stream_out;
    run(stream_in, stream_out);

    std::string my_answer = stream_out.str();
    std::ifstream t("output.txt");
    std::string expected_answer((std::istreambuf_iterator<char>(t)),
        std::istreambuf_iterator<char>());

    BOOST_ASSERT(boost::trim_right_copy(my_answer) == boost::trim_right_copy(expected_answer));
    return 0;
}
#endif
