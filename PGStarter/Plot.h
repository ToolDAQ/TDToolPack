#ifndef DAQ_PLOT_H
#define DAQ_PLOT_H

#include "PSQLInterface.h"

class Plot {
  public:
    Plot(std::string name, PSQLInterface& db, bool own = false);
    ~Plot();

    std::vector<float> getX() const { return getArray("x"); };
    float getX(size_t index)  const { return getArrayValue("x", index); };

    void setX(const std::vector<float>& x) { setArray("x", x); };
    void setX(size_t index, float x) { setArrayValue("x", index, x); };

    std::vector<float> getY() const { return getArray("y"); };
    float getY(size_t index)  const { return getArrayValue("y", index); };

    void setY(const std::vector<float>& y) { setArray("y", y); };
    void setY(size_t index, float y) { setArrayValue("y", index, y); };

    std::pair<std::vector<float>, std::vector<float>> getXY() const;
    std::pair<float, float> getXY(size_t index) const;

    void setXY(const std::vector<float>& x, const std::vector<float>& y);
    void setXY(size_t index, float x, float y);

    std::string getXLabel() const            { return getString("xlabel"); };
    void setXLabel(const std::string& label) { setString("xlabel", label); };

    std::string getYLabel() const            { return getString("ylabel"); };
    void setYLabel(const std::string& label) { setString("ylabel", label); };

    std::string getTitle() const             { return getString("title"); };
    void setTitle(const std::string& title)  { setString("title", title); };

    Store getInfo() const;
    void setInfo(Store&);
    void setInfo(const std::string& info) { setString("info", info); };

    // If we own a plot, we delete it when an instance of this class is
    // destroyed.
    bool own() const { return own_; };
    void setOwn(bool own) { own_ = own; };
    void disown() { own_ = false; };

  private:
    PSQLInterface& db;
    std::string id; // name, escaped for SQL
    bool own_;

    void  query0(const std::string&) const;
    Store query1(const std::string&) const;

    std::vector<float> getArray(const std::string& axis) const;
    void setArray(const std::string& axis, const std::vector<float>&);

    float getArrayValue(const std::string& axis, size_t index) const;
    void setArrayValue(const std::string& axis, size_t index, float value);

    std::string getString(const std::string& field) const;
    void setString(const std::string& field, const std::string& value);
};

#endif
