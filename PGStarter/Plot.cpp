#include "Plot.h"

// Put concat into utilities?
static 
std::stringstream&
concat_(std::stringstream& ss) {
  return ss;
};

template <typename T, typename... Rest>
std::stringstream&
concat_(std::stringstream& ss, const T& x, const Rest&... rest) {
  ss << x;
  return concat_(ss, rest...);
};

template <typename... Args>
std::string
concat(const Args&... args) {
  std::stringstream ss;
  concat_(ss, args...);
  return ss.str();
};

static std::string encodePGArray(const std::vector<float>& array) {
  std::stringstream ss;
  ss << "'{";
  bool first = true;
  for (float x : array) {
    if (!first) ss << ',';
    first = false;
    ss << x;
  };
  ss << "}'";
  return ss.str();
};

static std::vector<float> decodePGArray(const std::string& string, size_t length) {
  std::vector<float> result;
  result.reserve(length);

  std::stringstream ss(string);
  char c;
  ss >> c; // {
  while (ss && c != '}') {
    float x;
    ss >> x >> c;
    result.push_back(x);
  };
  return result;
};

static
void failQuery(
    const std::string& id,
    const std::string& query,
    const std::string& error
) {
  throw std::runtime_error(
      concat(
        "Plot `", id, "': query `", query, "' resulted in error `", error, '\''
      )
  );
};

Plot::Plot(std::string name, PSQLInterface& db, bool own):
  id(db.Quote(std::move(name))), db(db), own_(own)
{
  if (own_
      && !query1(concat("select true result from plots where plot = '", id, '\''))
          .Has("result"))
    // TODO: use "on conflict do nothing" when a more recent version of
    // PostgreSQL is available
    query0(concat("insert into plots (plot) values ('", id, "') "));
};

Plot::~Plot() {
  if (own_) db.SQLQuery(concat("delete from plots where plot = '", id, '\''));
  own_ = false;
};

void Plot::query0(const std::string& query) const {
  std::string error;
  if (!db.SQLQuery(query, &error)) failQuery(id, query, error);
};

Store Plot::query1(const std::string& query) const {
  std::string error;
  Store result;
  if (!db.SQLQuery(query, result, &error)) failQuery(id, query, error);
  return result;
};

std::vector<float> Plot::getArray(const std::string& axis) const {
  Store reply = query1(
      concat(
        "select array_length(", axis, ", 1) length ", axis,
        " from plots where plot = '", id, '\''
      )
  );
  return decodePGArray(*reply[axis], reply.Get<size_t>("length"));
};

void Plot::setArray(const std::string& axis, const std::vector<float>& array) {
  query0(
      concat(
        "update plots set ", axis, " = ", encodePGArray(array),
        " where plot = '", id, '\''
      )
  );
};

float Plot::getArrayValue(const std::string& axis, size_t index) const {
  Store reply = query1(
      concat("select ", axis, '[', index, "] from plots where plot = '", id, '\'')
  );
  // NOTE: returns 0 for indices out of range. Check?
  return reply.Get<float>(axis);
};

void Plot::setArrayValue(const std::string& axis, size_t index, float value) {
  query0(
      concat(
        "update plots set ", axis, '[', index, "] = ", value,
        " where plot = '", id, '\''
      )
  );
};

std::pair<std::vector<float>, std::vector<float>>
Plot::getXY() const {
  Store reply = query1(
      concat(
        "select array_length(x) xl, x, array_length(y), yl"
        " from plots where plot = '", id, '\''
      )
  );
  return {
    decodePGArray(*reply["x"], reply.Get<size_t>("xl")),
    decodePGArray(*reply["y"], reply.Get<size_t>("yl"))
  };
};

std::pair<float, float>
Plot::getXY(size_t index) const {
  Store reply = query1(
      concat(
        "select x[", index, "], y[", index, "] from plots"
        " where plot = '", id, '\''
      )
  );
  return { reply.Get<float>("x"), reply.Get<float>("y") };
};

void Plot::setXY(const std::vector<float>& x, const std::vector<float>& y) {
  query0(
      concat(
        "update plots set x = ", encodePGArray(x), ", y = ", encodePGArray(y),
        " where plot = '", id, '\''
      )
  );
};

void Plot::setXY(size_t index, float x, float y) {
  query0(
      concat(
        "update plots set x[", index, "] = ", x, ", y[", index, "] = ", y,
        " where plot = '", id, '\''
      )
  );
};

std::string Plot::getString(const std::string& field) const {
  return std::move(
      *query1(
        concat("select ", field, " from plots where plot = '", id, '\'')
      )[field]
  );
};

void Plot::setString(const std::string& field, const std::string& value) {
  query0(
      concat(
        "update plots set ", field, " = ", value, " where plot = '", id, '\''
      )
  );
};

Store Plot::getInfo() const {
  Store result;
  result.JsonParser(getString("info"));
  return result;
};

void Plot::setInfo(Store& info) {
  std::string string;
  info >> string;
  setString("info", string);
};
