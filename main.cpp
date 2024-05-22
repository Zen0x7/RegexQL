/**
 * RegexQL
 */

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>
#include <functional>

bool debug = false;

struct program_input {
    std::string key;
    std::vector<std::string> values;
};

std::vector<program_input> program_inputs;

void push_feature(std::string & key, std::string & value) {
    bool found = false;

    for (program_input & item : program_inputs) {
        if (item.key == key) {
            found = true;
            item.values.push_back(value);
            break;
        }
    }

    if (!found) {
        program_inputs.push_back({
            .key = key,
            .values = { value }
        });
    }
}

std::vector<std::string> get_values_of(const char * key) {
    for (const program_input & input : program_inputs) {
        if (input.key == key) {
            return input.values;
        }
    }
    throw std::invalid_argument( "key doesn't exists" );
}

bool include_input(const char * key) {
    for (const program_input & input : program_inputs) {
        if (input.key == key) {
            return true;
        }
    }
    return false;
}

bool is_input(const char * key) {
    std::vector<std::string> available_inputs =
        {"input", "query", "debug"};

    for (auto const & available_input : available_inputs) {
        if (available_input == key) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> keywords {
    "START",
    "VERB",
    "QUANTITY",
    "RULE",
    "IDENTIFIER",
    "FEATURE",
    "RELATION",
    "SYMBOL",
    "EMPLACER"
};

struct Procedure {
    enum Keywords {
        START, // 0 (defines the start of expression)
        VERB, // 1 (defines if something exists (affirmative|negative) (singular|plural))
        QUANTITY, // 2 (defines an amount to be considered)
        RULE, // 3 (defines validation rules)
        IDENTIFIER, // 4 (defines entities)
        FEATURE, // 5 (defines a glue)
        RELATION, // 6 (relation between previous and next)
        SYMBOL, // 7 (defines characters)
        EMPLACER, // 8 (emplace another expression)
        NEGATION, // 9 (denied of next)
    };
    Keywords keyword;
    std::string value;
};

std::map<std::string, Procedure::Keywords> available_keywords = {
    {"there", Procedure::Keywords::START },
    {"and", Procedure::Keywords::EMPLACER },
    {"is", Procedure::Keywords::VERB },
    {"are", Procedure::Keywords::VERB },
    {"can_be", Procedure::Keywords::VERB },
    {"alphanumeric", Procedure::Keywords::RULE },
    {"numeric", Procedure::Keywords::RULE },
    {"groups", Procedure::Keywords::IDENTIFIER },
    {"group", Procedure::Keywords::IDENTIFIER },
    {"character", Procedure::Keywords::IDENTIFIER },
    {"by", Procedure::Keywords::RELATION },
    {"using", Procedure::Keywords::RELATION },
    {"separated", Procedure::Keywords::FEATURE },
    {"joined", Procedure::Keywords::FEATURE },
    {"slash", Procedure::Keywords::SYMBOL },
    {"dot", Procedure::Keywords::SYMBOL },
    {"not", Procedure::Keywords::NEGATION },
};

std::map<std::string, std::string> available_symbols = {
    {"slash", "/" },
    {"dot", "." },
};

std::vector<std::string> expressions_modes = {
    {"DEFAULT"}, {"AFFIRMATIVE"}, {"NEGATIVE"}, {"OPTIONAL"}
};

enum ExpressionMode {
    DEFAULT,
    AFFIRMATIVE,
    NEGATIVE,
    OPTIONAL
};

struct ExpressionToken {
    Procedure::Keywords type;
    std::string value;
};

struct ExpressionNode {
    ExpressionNode * parent = nullptr;
    std::vector<ExpressionToken> tokens;
    std::vector<ExpressionNode *> childs;
    std::string value;
    ExpressionMode mode = { DEFAULT };
};

bool is_keyword(const char * key) {
    for (auto const & available_input : available_keywords) {
        if (available_input.first == key) {
            return true;
        }
    }
    return false;
}

Procedure::Keywords get_keyword(const char * key) {
    return available_keywords[key];
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void run(std::string & query) {
    std::vector<std::string> tokens;
    std::istringstream iss(query);

    for (std::string token; std::getline(iss, token, ' '); )
    {
        tokens.push_back(std::move(token));
    }

    std::vector<std::function<bool(std::string, std::vector<std::string>)>> lambdas;

    std::vector<Procedure> procedures;

    for (size_t position = 0; auto & token : tokens) {
        if (is_number(token)) {
            procedures.push_back({
                .keyword = Procedure::Keywords::QUANTITY,
                .value = token,
            });
        } else if (is_keyword(token.c_str())) {
            procedures.push_back({
                .keyword = get_keyword(token.c_str()),
                .value = token,
            });
        }
        position++;
        if (debug)
            std::cout << "- " << position << " " << token << std::endl;
    }

    // At this point, we have a vector with procedures and values
    // - 1 : 0 (START) : there
    // - 2 : 1 (VERB) : are
    // - 3 : 2 (QUANTITY) : 2
    // - 4 : 3 (RULE) : alphanumeric
    // - 5 : 4 (IDENTIFIER) : groups
    // - 6 : 5 (FEATURE) : separated
    // - 7 : 6 (RELATION): by
    // - 8 : 7 (SYMBOL): slash
    // - 9 : 8 (EMPLACER) : and

    if (debug) {
        std::cout << "" << std::endl;

        std::cout << "## procedures: " << std::endl;
        std::cout << "- position : type : keyword or value" << std::endl;
    }

    if (procedures.empty())
        throw std::invalid_argument( "Error (1000) Expression can't be empty" );

    std::vector<ExpressionNode> expressions;

    auto root = new ExpressionNode;
    auto current = new ExpressionNode;
    ExpressionMode last = { DEFAULT };

    bool slice = false;

    for (size_t position = 0; auto & procedure : procedures) {
        if (debug)
            std::cout << "- " << position + 1 << " : " << keywords[procedure.keyword] << " : " << procedure.value  << std::endl;

        position++;

        if (procedure.keyword == Procedure::Keywords::START) {
        } else if (procedure.keyword == Procedure::Keywords::VERB) {
            if (procedure.value == "are" || procedure.value == "is") {
                if (procedures[position + 1].keyword == Procedure::Keywords::NEGATION)
                    current->mode = NEGATIVE;
                else
                    current->mode = AFFIRMATIVE;
            } else {
                current->mode = OPTIONAL;
            }
            last = current->mode;
        } else if (procedure.keyword == Procedure::Keywords::EMPLACER) {
            slice = true;

            if (procedures[position + 1].keyword == Procedure::Keywords::NEGATION)
                current->mode = NEGATIVE;
            else
                current->mode = AFFIRMATIVE;

            last = current->mode;

            // auto item = new ExpressionNode;
            // current->nodes.push_back(item);
        } else {
            if (slice) {
                slice = false;
                root->childs.push_back(current);
                current = new ExpressionNode;
                if (last != DEFAULT)
                    current->mode = last;
            }

            auto extra = (position == procedures.size() ? "" : " ");

            current->value.append(procedure.value + extra);

            current->tokens.push_back({
                .type = procedure.keyword,
                .value = procedure.value,
            });
        }
    }

    root->childs.push_back(current);

    for (size_t i = 0; auto something : root->childs) {

        if (debug) {
            std::cout << "" << std::endl;
            std::cout << "## expressions for <" << something->value << "> mode <" << expressions_modes[something->mode] << ">" << std::endl;

            std::cout << "" << std::endl;

            std::cout << "### tokens" << std::endl;
            std::cout << "" << std::endl;
        }

        int quantity = 1;
        std::string identifier = "character";
        std::vector<std::string> rules;
        std::vector<std::string> features;

        for (size_t e = 0; auto & token : something->tokens) {
            if (debug)
                std::cout << "- " << keywords[token.type] << " : " << token.value << std::endl;

            if (e == 0 && token.type == Procedure::QUANTITY)
                quantity = std::stoi(token.value);

            if (token.type == Procedure::RULE)
                rules.push_back(token.value);

            if (token.type == Procedure::IDENTIFIER)
                identifier = token.value;

            if (token.type == Procedure::FEATURE)
                features.push_back(token.value);

            e++;
        }

        if (debug) {
            std::cout << "" << std::endl;
            std::cout << "#### details" << std::endl;
            std::cout << "" << std::endl;

            std::cout << "QUANTITY: " << quantity << std::endl;
            std::cout << "IDENTIFIER: " << identifier << std::endl;
        }

        if (!rules.empty()) {
            if (debug)
                std::cout << "RULES: " << std::endl;

            for (auto & rule : rules) {
                if (debug)
                    std::cout << "- " << rule << std::endl;
            }
        }

        if (!features.empty()) {
            if (debug)
                std::cout << "FEATURES: " << std::endl;

            for (auto & feature : features) {
                if (debug)
                    std::cout << "- " << feature << std::endl;
            }
        }

        std::string value = something->value;
        // run(value);
    }
}

void parse_and_run(std::string & query, std::vector<std::string> & inputs) {

    if (debug) {
        std::cout << "" << std::endl;
        std::cout << "# running on <" << query << ">" << std::endl;

        std::cout << "" << std::endl;
        std::cout << "# tokenization :" << std::endl;
    }

    // We need to convert all tokens into regex
    // By instance:
    // - <are> means <verify all the following as true> and finish with a identifiers (element, groups or group)
    //     - the next can be a number so <verify the count of the **next**>
    //     - the next can be a thing:
    //          - described with characteristics:
    //                 - can be alphanumeric
    //                 - can be numeric
    //                 - and more ...
    // - <groups|group|element> can be described with characteristics:
    //     - can be separated using a glue (space, dot, pipeline, dash, slash)
    //     - can be started by preffix
    //     - can be ended by suffix
    // - <or|and> are logic operators
    run(query);

    if (debug) {
        std::cout << "" << std::endl;
        std::cout << "# inputs :" << std::endl;
    }

    for (auto & input : inputs) {
        if (debug)
            std::cout << "- " << input << std::endl;
    }
}

/**
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char* argv[])
{
    std::vector<std::string> parameters;

    for (int position = 1; position < argc; position++) {
        parameters.emplace_back(argv[position]);
    }

    for (int position = 1; auto & parameter: parameters) {

        if (parameter.starts_with("--")) {
            size_t FEATURE = parameter.find('=');

            if (FEATURE == SIZE_MAX) {
                throw std::invalid_argument(
                    "Error (700) parameter " + std::to_string(position) + " should be equals separated key-value (--key=\"value\")"
                );
            }

            std::string key = parameter.substr(2, FEATURE - 2);
            std::string value = parameter.substr(FEATURE + 1, parameter.size() - FEATURE - 1);
            if (is_input(key.data())) {
                push_feature(key, value);
            } else {
                throw std::invalid_argument(
                    "Error (701) parameter '" + key + "' isn't allowed."
                );
            }
        } else {
            throw std::invalid_argument(
                "Error (700) parameter " + std::to_string(position) + " should starts with double dash (--key=\"value\")"
            );
        }
        position++;
    }

    bool has_query = include_input("query");
    bool has_input = include_input("input");

    debug = include_input("debug");

    if (!has_query || !has_input) {
        if (!has_query && !has_input) {
            throw std::invalid_argument("Error (300) both query and input are required parameters");
        }

        if (!has_input) {
            throw std::invalid_argument("Error (300) input is required parameter");
        }

        throw std::invalid_argument("Error (300) query is required parameter");
    }

    auto queries = get_values_of("query");
    auto inputs = get_values_of("input");

    if (debug) {
        std::cout << "" << std::endl;
        std::cout << "# queries: " << std::endl;
        for (auto & query : queries) {
            std::cout << "- " << query << std::endl;
        }

        std::cout << "" << std::endl;
        std::cout << "# inputs: " << std::endl;
        for (auto & input : inputs) {
            std::cout << "- " << input << std::endl;
        }
    }

    for (auto & query : queries) {
        parse_and_run(query, inputs);
    }

    return 0;
}
