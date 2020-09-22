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

#include "sqlite3_cpp11.h"

namespace kissqlite3 {

    std::ostream& operator<<(std::ostream& os, const query& q) {
        os << "SELECT (";

        bool comma = false;
        for (auto f : q.fields) {
            if (comma)
                os << ", ";
            comma = true;

            os << f;
        }
        os << ") FROM " << q.table;

        if (q.where_clause.size() > 0) {
            os << " WHERE (";

            comma = false;
            for (auto c : q.where_clause) {
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

        for (auto c : q.tail_clause) {
            os << " " << c.op;

            if (c.bind_type == BindType::sql_int)
                os << " " << c.num_value;
            else
                os << " " << c.value;
        }

        os << ";" << std::endl;

        if (q.where_clause.size() > 0) {
            for (auto c : q.where_clause) {
                if (c.op_only)
                    continue;

                os << "  bind_";
                
                switch (c.bind_type) {
                    case BindType::sql_blob:
                        os << "blob(\"" << c.value << "\", " << c.value.length() << ")";
                        break;
                    case BindType::sql_text:
                        os << "text(\"" << c.value << "\", " << c.value.length() << ")";
                        break;
                    case BindType::sql_int:
                        os << "int(" << (int) c.num_value << ")";
                        break;
                    case BindType::sql_int64:
                        os << "int64(" << (int64_t) c.num_value << ")";
                        break;
                    case BindType::sql_double:
                        os << "double(" << c.num_value << ")";
                        break;
                    case BindType::sql_null:
                        os << "null(nullptr);";
                        break;
                    case BindType::sql_joining_op:
                        break;
                };

                os << std::endl;
            }
        }

        return os;
    }

    // SELECT (x, y, z) FROM table
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields) {
        return query{db, table, fields};
    }

    // SELECT (x, y, z) FROM table WHERE (...)
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
            const std::list<query_element>& where_clause) {
        return query{db, table, fields, where_clause};
    }

    // SELECT (x, y, z) FROM table WHERE (...) LIMIT N
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
            const std::list<query_element>& where_clause,
            const _LIMIT& lim_op, int limit) {
        return query{db, table, fields, where_clause, {{lim_op, limit}}};
    }

    // SELECT (x, y, z) FROM table WHERE (...) ORDER BY f
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
            const std::list<query_element>& where_clause,
            const _ORDERBY& ord_op, const std::string& field) {
        return query{db, table, fields, where_clause, {{ord_op, field}}};
    }

    // SELECT (x, y, z) FROM table WHERE (...) ORDER BY f LIMIT n
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
            const std::list<query_element>& where_clause,
            const _ORDERBY& ord_op, const std::string& field,
            const _LIMIT& lim_op, int limit) {
        return query{db, table, fields, where_clause, 
            {{ord_op, field}, {lim_op, limit}}};
    }

    // SELECT (x, y, z) FROM table LIMIT N
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
            const _LIMIT& lim_op, int limit) {
        return query{db, table, fields, {}, {{lim_op, limit}}};
    }

    // SELECT (x, y, z) FROM table ORDER BY f
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
            const _ORDERBY& ord_op, const std::string& field) {
        return query{db, table, fields, {}, {{ord_op, field}}};
    }

    // SELECT (x, y, z) FROM table ORDER BY f LIMIT n
    query _SELECT(sqlite3 *db, const std::string& table, const std::list<std::string>& fields,
            const _ORDERBY& ord_op, const std::string& field,
            const _LIMIT& lim_op, int limit) {
        return query{db, table, fields, {}, 
            {{ord_op, field}, {lim_op, limit}}};
    }

    std::ostream& operator<<(std::ostream& os, const insert& i) {
        os << "INSERT INTO " << i.table << " (";

        bool comma = false;
        for (auto f : i.insert_fields) {
            if (comma)
                os << ", ";
            comma = true;

            os << f;
        }

        os << ") VALUES (";

        comma = false;
        for (auto t : i.insert_terms) {
            if (comma)
                os << ", ";
            comma = true;

            os << "?";
        }

        os << ");" << std::endl;

        for (auto t : i.insert_terms) {
            os << "  bind_";

            switch (t.bind_type) {
                case BindType::sql_blob:
                    os << "blob(\"" << t.value << "\", " << t.value.length() << ")";
                    break;
                case BindType::sql_text:
                    os << "text(\"" << t.value << "\", " << t.value.length() << ")";
                    break;
                case BindType::sql_int:
                    os << "int(" << (int) t.num_value << ")";
                    break;
                case BindType::sql_int64:
                    os << "int64(" << (int64_t) t.num_value << ")";
                    break;
                case BindType::sql_double:
                    os << "double(" << t.num_value << ")";
                    break;
                case BindType::sql_null:
                    os << "null(nullptr);";
                    break;
                case BindType::sql_joining_op:
                    break;
            };

            os << std::endl;
        }

        return os;
    }

    insert _INSERT(const std::string& table, const std::list<std::string>& fields,
            const std::list<insert_elem>& terms) {
        return insert(table, fields, terms);
    }

    std::ostream& operator<<(std::ostream& os, const update& q) {
        os << "UPDATE " << q.table << " SET ";

        bool comma = false;
        for (auto f : q.fields) {
            if (comma)
                os << ", ";
            comma = true;

            os << f << " = ?";
        }

        if (q.where_clause.size() > 0) {
            os << " WHERE (";

            comma = false;
            for (auto c : q.where_clause) {
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

        os << ";" << std::endl;

        for (auto t : q.terms) {
            os << "  bind_";

            switch (t.bind_type) {
                case BindType::sql_blob:
                    os << "blob(\"" << t.value << "\", " << t.value.length() << ")";
                    break;
                case BindType::sql_text:
                    os << "text(\"" << t.value << "\", " << t.value.length() << ")";
                    break;
                case BindType::sql_int:
                    os << "int(" << (int) t.num_value << ")";
                    break;
                case BindType::sql_int64:
                    os << "int64(" << (int64_t) t.num_value << ")";
                    break;
                case BindType::sql_double:
                    os << "double(" << t.num_value << ")";
                    break;
                case BindType::sql_null:
                    os << "null(nullptr);";
                    break;
                case BindType::sql_joining_op:
                    break;
            };

            os << std::endl;
        }

        if (q.where_clause.size() > 0) {
            for (auto c : q.where_clause) {
                if (c.op_only)
                    continue;

                os << "  bind_";
                
                switch (c.bind_type) {
                    case BindType::sql_blob:
                        os << "blob(\"" << c.value << "\", " << c.value.length() << ")";
                        break;
                    case BindType::sql_text:
                        os << "text(\"" << c.value << "\", " << c.value.length() << ")";
                        break;
                    case BindType::sql_int:
                        os << "int(" << (int) c.num_value << ")";
                        break;
                    case BindType::sql_int64:
                        os << "int64(" << (int64_t) c.num_value << ")";
                        break;
                    case BindType::sql_double:
                        os << "double(" << c.num_value << ")";
                        break;
                    case BindType::sql_null:
                        os << "null(nullptr);";
                        break;
                    case BindType::sql_joining_op:
                        break;
                };

                os << std::endl;
            }
        }
        return os;
    }

    update _UPDATE(const std::string& table, const std::list<std::string>& fields,
            const std::list<insert_elem>& terms) {
        return update(table, fields, terms);
    }

    update _UPDATE(const std::string& table, const std::list<std::string>& fields,
            const std::list<insert_elem>& terms, const std::list<query_element>& where_clause) {
        return update(table, fields, terms, where_clause);
    }

    // Extractors
    template<typename T>
    T sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column);

    template<>
    int sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column) {
        return (int) sqlite3_column_int(stmt, column);
    }

    template<>
    unsigned int sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column) {
        return (unsigned int) sqlite3_column_int(stmt, column);
    }

    template<>
    long sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column) {
        return (long) sqlite3_column_int64(stmt, column);
    }

    template<>
    unsigned long sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column) {
        return (unsigned long) sqlite3_column_int64(stmt, column);
    }

    template<>
    bool sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column) {
        return (bool) sqlite3_column_int(stmt, column);
    }

    template<>
    float sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column) {
        return (float) sqlite3_column_double(stmt, column);
    }

    template<>
    double sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column) {
        return sqlite3_column_double(stmt, column);
    }

    template<>
    std::string sqlite3_column_as(sqlite3_stmt *stmt, unsigned int column) {
        auto raw = (const char *) sqlite3_column_blob(stmt, column);
        auto len = sqlite3_column_bytes(stmt, column);
        return std::string(raw, len);
    }

};

