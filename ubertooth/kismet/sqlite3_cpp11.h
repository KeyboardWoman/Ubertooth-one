/*
    This file is part of Kismet

    Kismet is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kismet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Kismet; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// There are several sqlite3 to CPP11+; none of them seemed usable in this situation, or
// were incomplete and missing some crucial featuers, so this is yet another one which
// is limited and missing crucial features.
//
// Example usage:
//
//    auto q = _SELECT(db, "devices", 
//            {"devkey", "sourcemac", "last_signal"}, 
//            _WHERE("last_time", GT, 12345, 
//                AND, 
//                "max_signal", GT, -40, 
//                AND, 
//                "macaddr", LIKE, "aa:bb:cc:%"
//            ),
//            ORDERBY, "last_time",
//            LIMIT, 10);
//
//    auto i = _INSERT(db, "otherdevs", 
//            {"devkey", "packets", "data"},
//            {{"aa:bb:cc:dd:ee:ff"}, {1234}, {"some data"}});
//
//    auto u = _UPDATE(db, "update_target",
//            {"last_time", "packets"},
//            {{time(0)}, {15}},
//            _WHERE("devkey", EQ, "123456"));

#ifndef __SQLITE3_CPP11_H__
#define __SQLITE3_CPP11_H__

#include "config.h"

#include <vector>
#include <tuple>
#include <string>
#include <iostream>
#include <sstream>
#include <list>

#include <sqlite3.h>

namespace kissqlite3 {

    // Very fragile iterator wrapper; the stmt incremental processing doesn't allow us to have multiple views
    // into the iterator, so there's ONE begin which MUST be called as the first result, going backwards
    // won't work, and comparison only looks for end() because there's not really a way to know how many results
    // are in a return.
    //
    // Using this iterator anywhere other than
    // for (auto i : query) { ... } 
    // is likely dangerous.  
    class sqlite3_stmt_iterator {
        public:
            sqlite3_stmt_iterator(sqlite3_stmt *stmt) :
                stmt {stmt} { }
            sqlite3_stmt_iterator() :
                end {true} { }

            sqlite3_stmt_iterator& operator++() {
                auto r = sqlite3_step(stmt);

                if (r != SQLITE_ROW)
                    end = true;

                return *this;
            }

            bool operator==(const sqlite3_stmt_iterator& s) {
                return (end == s.end);
            }

            bool operator!=(const sqlite3_stmt_iterator& s) {
                return (end != s.end);
            }

            sqlite3_stmt *operator*() {
                return stmt;
            }

        protected:
            sqlite3_stmt *stmt = nullptr;
            bool end = false;
    };

    // Template compiletime grammar elements
    
    enum class BindType {
        sql_blob, sql_double, sql_int, sql_int64, sql_null, sql_text,
        // joining op (AND, OR) metatype
        sql_joining_op
    };

    typedef struct { std::string op = "AND"; } _AND;
    typedef struct { std::string op = "OR"; } _OR;

    static auto AND = _AND{};
    static auto OR = _OR{};

    typedef struct { std::string op = "<>"; } _NEQ;
    typedef struct { std::string op = "=="; } _EQ;

    static auto NEQ = _NEQ{};
    static auto EQ = _EQ{};

    typedef struct { std::string op = "<="; } _LE;
    typedef struct { std::string op = "<"; } _LT;
    typedef struct { std::string op = ">="; } _GE;
    typedef struct { std::string op = ">"; } _GT;

    static auto LE = _LE{};
    static auto LT = _LT{};
    static auto GE = _GE{};
    static auto GT = _GT{};

    typedef struct { std::string op = "LIKE"; } _LIKE;
    static auto LIKE = _LIKE{};

    typedef struct { std::string op = "ORDER_BY"; } _ORDERBY;
    static auto ORDERBY = _ORDERBY{};

    typedef struct { std::string op = "LIMIT"; } _LIMIT;
    static auto LIMIT = _LIMIT{};

    struct query_element {
        template<typename OT, typename T>
        query_element(const std::string& field, const OT& raw_op, T string_like_value) :
            field {field},
            bind_type {BindType::sql_text} {
            op = raw_op.op;
            std::stringstream ss;
            ss << string_like_value;
            value = ss.str();
        }

        template<typename OT>
        query_element(const std::string& field, const OT& raw_op, int num_value) :
            field {field},
            bind_type {BindType::sql_int},
            num_value{ (double) num_value } { 
                op = raw_op.op;
        }

        template<typename OT>
        query_element(const std::string& field, const OT& raw_op, unsigned int num_value) :
            field {field},
            bind_type {BindType::sql_int},
            num_value{ (double) num_value } { 
                op = raw_op.op;
        }

        template<typename OT>
        query_element(const std::string& field, const OT& raw_op, long int num_value) :
            field {field},
            bind_type {BindType::sql_int64},
            num_value{ (double) num_value } { 
                op = raw_op.op;
        }

        template<typename OT>
        query_element(const std::string& field, const OT& raw_op, unsigned long int num_value) :
            field {field},
            bind_type {BindType::sql_int64},
            num_value{ (double) num_value } { 
                op = raw_op.op;
        }

        template<typename OT>
        query_element(const std::string& field, const OT& raw_op, double num_value) :
            field {field},
            bind_type {BindType::sql_double},
            num_value{ (double) num_value } { 
                op = raw_op.op;
        }

        // Specific tail processing options
        query_element(const _ORDERBY& op, const std::string value) :
            op_value_only {true},
            op { op.op },
            value {value} { }

        query_element(const _LIMIT& op, int num_value) :
            op_value_only {true},
            op { op.op },
            bind_type {BindType::sql_int},
            num_value {(double) num_value} { }

        query_element(const _AND& op) :
            op_only {true},
            op {op.op},
            bind_type {BindType::sql_joining_op} { }

        query_element(const _OR& op) :
            op_only {true},
            op {op.op},
            bind_type {BindType::sql_joining_op} { }

        std::string field;

        bool op_value_only = false;
        bool op_only = false;
        std::string op;

        BindType bind_type;
        std::string value;
        double num_value;
    };

    template<typename OP, typename... Args>
    void _WHERE(std::list<query_element>& vec);

    // X <op> VALUE
    template<typename OP, typename VL, typename... Args>
    void _WHERE(std::list<query_element>& vec, const std::string& field, 
            const OP& op, const VL value);

    // JOINER X <op> VALUE {...}
    template<typename JN, typename OP, typename VL, typename... Args>
    void _WHERE(std::list<query_element>& vec, const OP& join,
            const std::string& field, const JN& op, const VL value);

    template<typename JN, typename OP, typename VL, typename... Args>
    void _WHERE(std::list<query_element>& vec, const JN& join, const std::string& field, const OP& op, 
            const VL value, const Args& ... args);

    template<typename OP, typename VL, typename... Args>
    std::list<query_element> _WHERE(const std::string& field, const OP& op, const VL value);

    template<typename OP, typename VL, typename... Args>
    std::list<query_element> _WHERE(const std::string& field, const OP& op, const VL value,
            const Args& ... args);

    class query {
    public:
        query(sqlite3 *db, const std::string& table, const std::list<std::string>& fields) :
            db {db},
            table {table},
            fields {fields} { }

        query(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
                const std::list<query_element>& where_clause) : 
            db {db},
            table {table},
            fields {fields},
            where_clause {where_clause} { }

        query(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
                const std::list<query_element>& where_clause,
                const std::list<query_element>& tail_clause) : 
            db {db},
            table {table},
            fields {fields},
            where_clause {where_clause},
            tail_clause {tail_clause} { }

        void append_where(const _AND& join_and, const std::list<query_element>& additional_clauses) {
            if (where_clause.size() > 0)
                where_clause.push_back(AND);
            where_clause.insert(where_clause.end(), additional_clauses.begin(), additional_clauses.end());
        }

        void append_where(const _OR& join_r, const std::list<query_element>& additional_clauses) {
            if (where_clause.size() > 0)
                where_clause.push_back(OR);
            where_clause.insert(where_clause.end(), additional_clauses.begin(), additional_clauses.end());
        }

        void append_clause(const _LIMIT& limit_r, int limit) {
            tail_clause.push_back(query_element{LIMIT, limit});
        }

        void append_clause(const _ORDERBY& order_r, const std::string& field) {
            tail_clause.push_back(query_element{ORDERBY, field});
        }

        void bind_stmt() {
            if (stmt != nullptr)
                sqlite3_finalize(stmt);

            // Generate the placeholdered WHERE string
            std::stringstream os;

            os << "SELECT ";

            bool comma = false;
            for (auto f : fields) {
                if (comma)
                    os << ", ";
                comma = true;

                os << f;
            }
            os << " FROM " << table;

            if (where_clause.size() > 0) {
                os << " WHERE (";

                comma = false;
                for (auto c : where_clause) {
                    if (c.op_only) {
                        os << " " << c.op << " ";
                        comma = false;
                        continue;
                    }

                    if (comma) 
                        os << ", ";
                    comma = true;

                    os << c.field << " " << c.op << " ?";
                }
                os << ")";
            }

            for (auto c : tail_clause) {
                os << " " << c.op;

                if (c.bind_type == BindType::sql_int)
                    os << " " << c.num_value;
                else
                    os << " " << c.value;
            }

            int r;
            const char *pz = nullptr;

            r = sqlite3_prepare(db, os.str().c_str(), os.str().length(), &stmt, &pz);

            if (r != SQLITE_OK)
                throw std::runtime_error("Failed to prepare statement: " + os.str() + " " + 
                        std::string(sqlite3_errmsg(db)));

            // Bind all the values
            unsigned int bind_pos = 1;
            for (auto c : where_clause) {
                if (c.op_only)
                    continue;

                switch (c.bind_type) {
                    case BindType::sql_blob:
                        sqlite3_bind_blob(stmt, bind_pos++, c.value.data(), c.value.length(), SQLITE_TRANSIENT);
                        break;
                    case BindType::sql_text:
                        sqlite3_bind_text(stmt, bind_pos++, c.value.data(), c.value.length(), SQLITE_TRANSIENT);
                        break;
                    case BindType::sql_int:
                        sqlite3_bind_int(stmt, bind_pos++, c.num_value);
                        break;
                    case BindType::sql_int64:
                        sqlite3_bind_int64(stmt, bind_pos++, c.num_value);
                        break;
                    case BindType::sql_double:
                        sqlite3_bind_double(stmt, bind_pos++, c.num_value);
                        break;
                    case BindType::sql_null:
                        sqlite3_bind_null(stmt, bind_pos++);
                        break;
                    case BindType::sql_joining_op:
                        break;
                };
            }

            r = sqlite3_reset(stmt);
            if (r != SQLITE_OK)
                throw std::runtime_error("Failed to prepare statement to execute: " + os.str() + " " +
                        std::string(sqlite3_errmsg(db)));
        }

        // Get the begin and 'end' iterators; the begin iterator executes the first step of
        // the statement after binding it.
        sqlite3_stmt_iterator begin() {
            if (db == nullptr)
                return sqlite3_stmt_iterator();

            bind_stmt();
            auto r = sqlite3_step(stmt);

            if (r != SQLITE_ROW)
                return sqlite3_stmt_iterator();

            return sqlite3_stmt_iterator(stmt);
        }

        sqlite3_stmt_iterator end() {
            return sqlite3_stmt_iterator();
        }

        sqlite3 *db = nullptr;
        sqlite3_stmt *stmt = nullptr;

        std::string table;
        std::list<std::string> fields;
        std::list<query_element> where_clause;
        std::list<query_element> tail_clause;

        friend std::ostream& operator<<(std::ostream& os, const query& q);
    };

    template<typename OP, typename... Args>
    void _WHERE(std::list<query_element>& vec) { }

    // X <op> VALUE
    template<typename OP, typename VL, typename... Args>
    void _WHERE(std::list<query_element>& vec, const std::string& field, 
            const OP& op, const VL value) {
        vec.push_back(query_element{field, op, value});
    }

    // JOINER X <op> VALUE {...}
    template<typename JN, typename OP, typename VL, typename... Args>
    void _WHERE(std::list<query_element>& vec, const OP& join,
            const std::string& field, const JN& op, const VL value) {
        vec.push_back(query_element{join});
        _WHERE(vec, field, op, value);
    }

    template<typename JN, typename OP, typename VL, typename... Args>
    void _WHERE(std::list<query_element>& vec, const JN& join, const std::string& field, const OP& op, 
            const VL value, const Args& ... args) {
        _WHERE(vec, join, field, op, value);
        _WHERE(vec, args...);
    }

    template<typename OP, typename VL, typename... Args>
    std::list<query_element> _WHERE(const std::string& field, const OP& op, const VL value) {
        auto ret = std::list<query_element>{};

        _WHERE(ret, field, op, value);

        return ret;
    }

    template<typename OP, typename VL, typename... Args>
    std::list<query_element> _WHERE(const std::string& field, const OP& op, const VL value,
            const Args& ... args) {
        auto ret = std::list<query_element>{};

        _WHERE(ret, field, op, value);
        _WHERE(ret, args...);

        return ret;
    }
    
    std::ostream& operator<<(std::ostream& os, const query& q);

    // SELECT (x, y, z) FROM table
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields);

    // SELECT (x, y, z) FROM table WHERE (...)
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
            const std::list<query_element>& where_clause);

    // SELECT (x, y, z) FROM table WHERE (...) LIMIT N
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
            const std::list<query_element>& where_clause,
            const _LIMIT& lim_op, int limit);

    // SELECT (x, y, z) FROM table WHERE (...) ORDER BY f
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
            const std::list<query_element>& where_clause,
            const _ORDERBY& ord_op, const std::string& field);

    // SELECT (x, y, z) FROM table WHERE (...) ORDER BY f LIMIT n
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
            const std::list<query_element>& where_clause,
            const _ORDERBY& ord_op, const std::string& field,
            const _LIMIT& lim_op, int limit);

    // SELECT (x, y, z) FROM table LIMIT N
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
            const _LIMIT& lim_op, int limit);

    // SELECT (x, y, z) FROM table ORDER BY f
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
            const _ORDERBY& ord_op, const std::string& field);

    // SELECT (x, y, z) FROM table ORDER BY f LIMIT n
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
            const _ORDERBY& ord_op, const std::string& field,
            const _LIMIT& lim_op, int limit);

    struct insert_elem {
        insert_elem(const std::string& value) :
            bind_type {BindType::sql_text},
            value {value} { }

        insert_elem(int value) :
            bind_type {BindType::sql_int},
            num_value {(double) value} { }

        insert_elem(unsigned int value) :
            bind_type {BindType::sql_int},
            num_value {(double) value} { }

        insert_elem(long int value) :
            bind_type {BindType::sql_int64},
            num_value {(double) value} { }

        insert_elem(unsigned long int value) :
            bind_type {BindType::sql_int64},
            num_value {(double) value} { }

        insert_elem(float value) :
            bind_type {BindType::sql_double},
            num_value {value} { }

        insert_elem(double value) :
            bind_type {BindType::sql_double},
            num_value {value} { }

        BindType bind_type;
        std::string value;
        double num_value;

        friend std::ostream& operator<<(std::ostream& os, const insert_elem& e);
    };

    struct insert {
        insert(const std::string& table, const std::list<std::string>& fields, 
                const std::list<insert_elem>& terms) :
            table {table},
            insert_fields {fields},
            insert_terms {terms} {
                if (fields.size() != terms.size())
                    throw std::runtime_error("Supplied fields not equal to supplied template");
            }

        std::string table;
        std::list<std::string> insert_fields;
        std::list<insert_elem> insert_terms;

        friend std::ostream& operator<<(std::ostream& os, const insert& i);
    };

    std::ostream& operator<<(std::ostream& os, const insert& i);

    insert _INSERT(const std::string& table, const std::list<std::string>& fields,
            const std::list<insert_elem>& terms);

    struct update {
        update(const std::string& table, const std::list<std::string>& fields, 
                const std::list<insert_elem>& terms) :
            table {table},
            fields {fields},
            terms {terms} {
                if (fields.size() != terms.size()) {
                    throw std::runtime_error("Supplied fields not equal to supplied template");
                }
            }

        update(const std::string& table, const std::list<std::string>& fields,
                const std::list<insert_elem>& terms,
                const std::list<query_element>& where_clause) : 
            table {table},
            fields {fields},
            terms {terms},
            where_clause {where_clause} { 
                if (fields.size() != terms.size()) {
                    throw std::runtime_error("Supplied fields not equal to supplied template");
                }
            }

        std::string table;
        std::list<std::string> fields;
        std::list<insert_elem> terms;
        std::list<query_element> where_clause;

        friend std::ostream& operator<<(std::ostream& os, const update& q);
    };

    std::ostream& operator<<(std::ostream& os, const update& q);

    update _UPDATE(const std::string& table, const std::list<std::string>& fields,
            const std::list<insert_elem>& terms);

    update _UPDATE(const std::string& table, const std::list<std::string>& fields,
            const std::list<insert_elem>& terms, const std::list<query_element>& where_clause);

    // Simple column extractors
    template<typename T>
    T sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column);

    template<>
    int sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column);

    template<>
    unsigned int sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column);

    template<>
    long sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column);

    template<>
    unsigned long sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column);

    template<>
    bool sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column);

    template<>
    float sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column);

    template<>
    double sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column);

    template<>
    std::string sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column);
};

#endif
