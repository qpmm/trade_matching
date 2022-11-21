#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

// Constants and typedefs
const bool SELL_SIDE = false;
const bool BUY_SIDE = true;

using TraderType = std::string;
using QuantityType = unsigned int;
using PriceType = unsigned int;

// Internal representation of a resting order in the market
struct InternalOrder
{
    TraderType trader_id;
    PriceType price;
};

template<class Comp>
using InternalOrders = std::multimap<InternalOrder, QuantityType, Comp>;

// Representation of a trade or aggressor's order
struct Order : InternalOrder
{
    bool side;
    QuantityType quantity;

    inline bool exhausted()
    {
        return (quantity == 0);
    }
};

using Trades = std::vector<Order>;

// Selling orders are sorted by price from lowest to highest,
// so that the best price for the buyer is in the beginning
struct SellComp
{
bool operator()(const InternalOrder& o1, const InternalOrder& o2) const
{
return (o1.price < o2.price);
}
};

// Buying orders are sorted by price from highest to lowest,
// so that the best price for the seller is in the beginning
struct BuyComp
{
bool operator()(const InternalOrder& o1, const InternalOrder& o2) const
{
return (o1.price > o2.price);
}
};

// Trades are sorted by trader, side and price
struct TradeComp
{
bool operator()(const Order& o1, const Order& o2) const
{
return std::tie(o1.trader_id, o1.side, o1.price) < std::tie(o2.trader_id, o2.side, o2.price);
}
};

class Market
{
public:
Market(){}

Trades execute_order(Order order)
{
Trades trades;

if (order.side == BUY_SIDE)
trades = _execute_order(sell_orders, buy_orders, order);
else
trades = _execute_order(buy_orders, sell_orders, order);

if (!trades.empty())
{
// Trades are sorted by trader, side and price
std::sort(trades.begin(), trades.end(), TradeComp());

if (trades.size() > 2)
    merge_trades(trades);
}

return trades;
}

Trades _execute_order(auto& opposing_orders, auto& same_side_orders, Order& aggr_order)
{
Trades trades;

if (opposing_orders.empty())
{
same_side_orders.insert({aggr_order, aggr_order.quantity});
return trades;
}

auto price_acceptable = std::not_fn(same_side_orders.key_comp()); // using std::not_fn for better readability
int exhausted_rest_orders = 0;

for (auto& [rest_order, rest_quantity]: opposing_orders)
{
if (!price_acceptable(rest_order, aggr_order) || aggr_order.exhausted())
    break;

auto overlap_quantity = std::min(rest_quantity, aggr_order.quantity);
aggr_order.quantity -= overlap_quantity;
if (rest_quantity -= overlap_quantity; rest_quantity == 0)
    exhausted_rest_orders++;

trades.push_back({aggr_order.trader_id, rest_order.price, aggr_order.side, overlap_quantity});
trades.push_back({rest_order.trader_id, rest_order.price, !aggr_order.side, overlap_quantity});
}

if (exhausted_rest_orders)
opposing_orders.erase(
    opposing_orders.begin(),
    std::next(opposing_orders.begin(), exhausted_rest_orders)
);

if (!aggr_order.exhausted())
same_side_orders.insert({aggr_order, aggr_order.quantity});

return trades;
}

/*
* Several trades of one trader with the same side and price,
* created on one aggressor execution, should be reported as one trade with cumulative size.
*/
void merge_trades(Trades& trades)
{
auto may_be_merged = std::not_fn(TradeComp());  // using std::not_fn for better readability
for (auto merge_begin = trades.begin(); merge_begin != trades.end();)
{
auto merge_end = std::next(merge_begin);
for (; merge_end != trades.end() && may_be_merged(*merge_begin, *merge_end); merge_end++)
    merge_begin->quantity += merge_end->quantity;

merge_begin = trades.erase(std::next(merge_begin), merge_end);
}
}

private:
InternalOrders<BuyComp> buy_orders;
InternalOrders<SellComp> sell_orders;
};

void print_trades(const Trades& trades)
{
for (const auto& t: trades)
std::cout << t.trader_id << (t.side == BUY_SIDE ? '+': '-') << t.quantity << '@' << t.price << " ";

std::cout << std::endl;
}

int main()
{
Trades orders;

std::string line;
for (std::getline(std::cin, line); !line.empty(); std::getline(std::cin, line))
{
Order order;
char side;

std::stringstream parser(line);
parser >> order.trader_id >> side >> order.quantity >> order.price;
order.side = (side == 'B');

orders.push_back(order);
}

Market market;
Trades trades;
for (const auto& order: orders)
if (trades = market.execute_order(order); !trades.empty())
print_trades(trades);

return 0;
}
