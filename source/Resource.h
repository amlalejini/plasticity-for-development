#ifndef _RESOURCE_H
#define _RESOURCE_H

enum class ResourceType { STATIC, PERIODIC };

class Resource {
public:
  static constexpr double MIN_RESOURCE_AMOUNT = 0.1;

protected:
  size_t resource_id=(size_t)-1; ///< Resource identifier
  ResourceType type=ResourceType::STATIC;
  double amount=0;               ///< How much resource is available?
  bool available=false;          ///< Is the resource available?
  size_t time_in_state=0;        ///< How long has this resource been in its current state?

public:
  /// Set resource type
  void SetType(ResourceType t) { type = t; }

  /// Set resource ID
  void SetID(size_t id) { resource_id = id; }

  /// Reset this resource
  void Reset();

  /// Is this resource available?
  bool IsAvailable() const { return available; }

  /// Get resource type
  ResourceType GetType() const { return type; }

  /// Get resource ID
  size_t GetID() const { return resource_id; }

  /// Get amount of available resource
  double GetAmount() const { return amount; }

  /// Get the time this resource has been available (0 if unavailable)
  size_t GetTimeAvailable() const { return (available) ? time_in_state : 0; }

  /// Get the time that this resource has been unavailble (0 if available)
  size_t GetTimeUnavailable() const { return (!available) ? time_in_state : 0; }

  /// Attempt to consume an amount of this resource
  /// Returns amount consumed
  double ConsumeFixed(double value);

  /// Attempt to consume a fixed proportion of this resource
  /// Returns amount consumed
  double ConsumeProportion(double prop);

  /// Decay a fixed amount of this resource
  void DecayFixed(double value);

  /// Decay a proportion of this resource
  void DecayProportion(double prop);

  /// Manipulate the amount of resource
  void SetAmount(double value);

  /// Increment the amount of resource available
  void IncAmount(double value);

  /// Advance resource availability tracking by a single time step (i.e., how long
  /// has this resource been available/unavailable?)
  void AdvanceAvailabilityTracking();
};

/// Reset this resource
void Resource::Reset() {
  amount = 0;
  available = false;
  time_in_state = 0;
}

/// Attempt to consume an amount of this resource
double Resource::ConsumeFixed(double value) {
  double consumed = 0;
  if (value > amount) {
    // Requesting more resource than available
    // - Consume everything.
    consumed = amount;
    amount = 0.0;
  } else {
    // Consume requested amount
    consumed = value;
    // If consumption would push amount below min threshold, set to 0; otherwise, subtract consumed amount
    amount = ((amount - value) < MIN_RESOURCE_AMOUNT) ? 0.0 : amount - value;
  }
  // Did resource transition from available => unavailable as a result of this
  // consumption?
  if (available && amount == 0.0) {
    available = false;
    time_in_state = 0;
  }
  // return amount consumed
  return consumed;
}

/// Attempt to consume a fixed proportion
double Resource::ConsumeProportion(double prop) {
  double consumed = prop*amount;
  amount -= consumed;
  if (amount < MIN_RESOURCE_AMOUNT) amount = 0.0;
  // Did resource transition from available => unavailable as a result of this
  // consumption?
  if (available && amount == 0.0) {
    available = false;
    time_in_state = 0;
  }
  return consumed;
}

/// Decay a fixed amount of this resource
void Resource::DecayFixed(double value) {
  if (value > amount) {
    // Decaying more resource than available, decay all.
    amount = 0.0;
  } else {
    // Decay the requested amount
    amount -= value;
  }
  // Is resource level below the minimum threshold?
  if (amount < MIN_RESOURCE_AMOUNT) amount = 0.0;
  // did resource transition?
  if (available && amount == 0.0) {
    available = false;
    time_in_state = 0;
  }
}

/// Decay a proportion of this resource
void Resource::DecayProportion(double prop) {
  amount -= prop*amount;
  if (amount < MIN_RESOURCE_AMOUNT) amount = 0.0;
  // did the resource transition?
  if (available && amount == 0.0) {
    available = false;
    time_in_state = 0;
  }
}

/// Manipulate the amount of resource
void Resource::SetAmount(double value) {
  emp_assert(value >= 0);
  amount = (value < MIN_RESOURCE_AMOUNT) ? 0.0 : value;
  // If resource availability transitioned (available <===> unavailable), adjust
  // internal tracking variables
  if (amount == 0.0 && available) { // Resource changed state: available => unavailable
    time_in_state = 0;
    available = false;
  } else if (amount > 0.0 && !available) { // Resource changed state: unavailable => available
    time_in_state = 0;
    available = true;
  }
}

/// Increment the amount of resource
void Resource::IncAmount(double value) {
  emp_assert(amount + value >= 0);
  amount += value;
  if (amount < MIN_RESOURCE_AMOUNT) amount = 0.0;
  // If resource availability transitioned (available <===> unavailable), adjust
  // internal tracking variables
  if (amount == 0.0 && available) { // Resource changed state: available => unavailable
    time_in_state = 0;
    available = false;
  } else if (amount > 0.0 && !available) { // Resource changed state: unavailable => available
    time_in_state = 0;
    available = true;
  }
}

/// Advance resource availability tracking by a single time step (i.e., how long
/// has this resource been available/unavailable?)
void Resource::AdvanceAvailabilityTracking() {
  time_in_state++;
}

#endif