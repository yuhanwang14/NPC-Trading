#pragma once

#include <string>
#include <utility>

#include "common.hpp"
#include "message_bus.hpp"

namespace npcTrading
{

  class Message;
  class MarketDataMessage;

  // ============================================================================
  // Instrument Specification
  // ============================================================================

  class Instrument
  {
   public:
    Instrument() = default;
    Instrument(
        InstrumentId id,
        const std::string& symbol,
        VenueId venue,
        double tick_size,
        double step_size,
        double min_quantity,
        double max_quantity,
        Timestamp timestamp = std::chrono::system_clock::now())
        : id_(id),
          symbol_(symbol),
          venue_(venue),
          tick_size_(tick_size),
          step_size_(step_size),
          min_quantity_(min_quantity),
          max_quantity_(max_quantity),
          timestamp_(timestamp)
    {
    }

    InstrumentId id() const
    {
      return id_;
    }
    std::string symbol() const
    {
      return symbol_;
    }
    VenueId venue() const
    {
      return venue_;
    }
    double tick_size() const
    {
      return tick_size_;
    }
    double step_size() const
    {
      return step_size_;
    }
    double min_quantity() const
    {
      return min_quantity_;
    }
    double max_quantity() const
    {
      return max_quantity_;
    }
    Timestamp timestamp() const
    {
      return timestamp_;
    }

   private:
    InstrumentId id_;
    std::string symbol_;
    VenueId venue_;
    double tick_size_;     // Minimum price increment
    double step_size_;     // Minimum quantity increment
    double min_quantity_;  // Minimum order size
    double max_quantity_;  // Maximum order size
    Timestamp timestamp_;
  };

  // ============================================================================
  // Market Data Types
  // ============================================================================

  /// Quote tick (best bid/ask)
  class QuoteTick
  {
   public:
    QuoteTick() = default;
    QuoteTick(
        InstrumentId instrument_id,
        Price bid_price,
        Price ask_price,
        Quantity bid_size,
        Quantity ask_size,
        Timestamp timestamp)
        : instrument_id_(instrument_id),
          bid_price_(bid_price),
          ask_price_(ask_price),
          bid_size_(bid_size),
          ask_size_(ask_size),
          timestamp_(timestamp)
    {
    }

    InstrumentId instrument_id() const
    {
      return instrument_id_;
    }
    Price bid_price() const
    {
      return bid_price_;
    }
    Price ask_price() const
    {
      return ask_price_;
    }
    Quantity bid_size() const
    {
      return bid_size_;
    }
    Quantity ask_size() const
    {
      return ask_size_;
    }
    Timestamp timestamp() const
    {
      return timestamp_;
    }

   private:
    InstrumentId instrument_id_;
    Price bid_price_;
    Price ask_price_;
    Quantity bid_size_;
    Quantity ask_size_;
    Timestamp timestamp_;
  };

  /// Trade tick (executed trade)
  class TradeTick
  {
   public:
    TradeTick() = default;
    TradeTick(InstrumentId instrument_id, Price price, Quantity size, OrderSide aggressor_side, Timestamp timestamp)
        : instrument_id_(instrument_id),
          price_(price),
          size_(size),
          aggressor_side_(aggressor_side),
          timestamp_(timestamp)
    {
    }

    InstrumentId instrument_id() const
    {
      return instrument_id_;
    }
    Price price() const
    {
      return price_;
    }
    Quantity size() const
    {
      return size_;
    }
    OrderSide aggressor_side() const
    {
      return aggressor_side_;
    }
    Timestamp timestamp() const
    {
      return timestamp_;
    }

   private:
    InstrumentId instrument_id_;
    Price price_;
    Quantity size_;
    OrderSide aggressor_side_;
    Timestamp timestamp_;
  };

  /// Bar (OHLCV candle)
  class Bar
  {
   public:
    Bar() = default;
    Bar(BarType bar_type, Price open, Price high, Price low, Price close, Quantity volume, Timestamp timestamp)
        : bar_type_(bar_type),
          open_(open),
          high_(high),
          low_(low),
          close_(close),
          volume_(volume),
          quote_volume_(0),
          num_trades_(0),
          timestamp_(timestamp)
    {
    }

    // Extended constructor with quote_volume and num_trades
    Bar(BarType bar_type,
        Price open,
        Price high,
        Price low,
        Price close,
        Quantity volume,
        Quantity quote_volume,
        int64_t num_trades,
        Timestamp timestamp)
        : bar_type_(bar_type),
          open_(open),
          high_(high),
          low_(low),
          close_(close),
          volume_(volume),
          quote_volume_(quote_volume),
          num_trades_(num_trades),
          timestamp_(timestamp)
    {
    }

    BarType bar_type() const
    {
      return bar_type_;
    }
    Price open() const
    {
      return open_;
    }
    Price high() const
    {
      return high_;
    }
    Price low() const
    {
      return low_;
    }
    Price close() const
    {
      return close_;
    }
    Quantity volume() const
    {
      return volume_;
    }
    Quantity quote_volume() const
    {
      return quote_volume_;
    }
    int64_t num_trades() const
    {
      return num_trades_;
    }
    Timestamp timestamp() const
    {
      return timestamp_;
    }

   private:
    BarType bar_type_;
    Price open_;
    Price high_;
    Price low_;
    Price close_;
    Quantity volume_;
    Quantity quote_volume_;
    int64_t num_trades_{ 0 };
    Timestamp timestamp_;
  };

  /// Order book level
  struct OrderBookLevel
  {
    Price price;
    Quantity size;
    int order_count;
  };

  /// Order book (depth snapshot)
  class OrderBook
  {
   public:
    OrderBook() = default;
    OrderBook(
        InstrumentId instrument_id,
        const std::vector<OrderBookLevel>& bids,
        const std::vector<OrderBookLevel>& asks,
        Timestamp timestamp)
        : instrument_id_(instrument_id),
          bids_(bids),
          asks_(asks),
          timestamp_(timestamp)
    {
    }

    InstrumentId instrument_id() const
    {
      return instrument_id_;
    }
    const std::vector<OrderBookLevel>& bids() const
    {
      return bids_;
    }
    const std::vector<OrderBookLevel>& asks() const
    {
      return asks_;
    }
    Timestamp timestamp() const
    {
      return timestamp_;
    }

    Price best_bid_price() const
    {
      return bids_.empty() ? Price(0) : bids_[0].price;
    }
    Price best_ask_price() const
    {
      return asks_.empty() ? Price(0) : asks_[0].price;
    }

   private:
    InstrumentId instrument_id_;
    std::vector<OrderBookLevel> bids_;
    std::vector<OrderBookLevel> asks_;
    Timestamp timestamp_;
  };

  // ============================================================================
  // Market Data Messages (for MessageBus transport)
  // ============================================================================

  class MarketDataMessage : public Message
  {
   public:
    virtual void dispatch_to(class Actor& actor) const = 0;
  };

  class QuoteTickMessage : public MarketDataMessage
  {
   public:
    explicit QuoteTickMessage(QuoteTick tick) : tick_(std::move(tick))
    {
    }

    Timestamp timestamp() const override
    {
      return tick_.timestamp();
    }
    std::string type() const override
    {
      return "QuoteTick";
    }
    const QuoteTick& tick() const
    {
      return tick_;
    }
    void dispatch_to(class Actor& actor) const override;

   private:
    QuoteTick tick_;
  };

  class TradeTickMessage : public MarketDataMessage
  {
   public:
    explicit TradeTickMessage(TradeTick tick) : tick_(std::move(tick))
    {
    }

    Timestamp timestamp() const override
    {
      return tick_.timestamp();
    }
    std::string type() const override
    {
      return "TradeTick";
    }
    const TradeTick& trade() const
    {
      return tick_;
    }
    void dispatch_to(class Actor& actor) const override;

   private:
    TradeTick tick_;
  };

  class BarMessage : public MarketDataMessage
  {
   public:
    explicit BarMessage(Bar bar) : bar_(std::move(bar))
    {
    }

    Timestamp timestamp() const override
    {
      return bar_.timestamp();
    }
    std::string type() const override
    {
      return "Bar";
    }
    const Bar& bar() const
    {
      return bar_;
    }
    void dispatch_to(class Actor& actor) const override;

   private:
    Bar bar_;
  };

  class OrderBookMessage : public MarketDataMessage
  {
   public:
    explicit OrderBookMessage(OrderBook book) : book_(std::move(book))
    {
    }

    Timestamp timestamp() const override
    {
      return book_.timestamp();
    }
    std::string type() const override
    {
      return "OrderBook";
    }
    const OrderBook& book() const
    {
      return book_;
    }
    void dispatch_to(class Actor& actor) const override;

   private:
    OrderBook book_;
  };

}  // namespace npcTrading
