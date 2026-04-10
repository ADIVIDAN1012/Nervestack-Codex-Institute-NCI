"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = activate;
exports.deactivate = deactivate;
const vscode = require("vscode");
const keywords = [
    "spec", "funcall", "forward", "check", "alter", "altern", "until", "traverse", "ask", "show", "erase", "ignore", "skip", "spill", "convert", "off", "on", "both", "either", "no", "source", "universal", "within", "equal", "attempt", "trap", "conclude", "authen", "condense", "tag", "solve", "private", "protected", "public", "internal", "expose", "paral", "hold", "flux", "barrier", "permit", "signal", "inlet", "fetch", "modify", "seal", "kind", "adopt", "father", "child", "slip", "wipe", "transform", "den", "pack", "unpack", "listen", "trigger", "track", "trace", "watch", "plug", "share", "toolkit", "bloc", "embed", "bridge", "link", "belong", "infuse", "peek", "assert", "skelet", "decon", "from", "to", "nick", "blueprint"
];
const builtins = [
    "kind", "avail", "exist", "output", "absol", "Roff", "exponent", "maximum", "minimum", "length", "reverse", "sort", "to_text", "to_list", "apply", "map", "filter", "props", "methods", "time_now", "halt", "ask", "tag"
];
const allKeywords = [...keywords, ...builtins];
const keywordDescriptions = {
    "spec": "Defines a function.",
    "blueprint": "Defines a class or a blueprint for objects.",
    "adopt": "Inherits from a parent blueprint.",
    "trap": "Catches an error in an `attempt` block.",
    "attempt": "Starts a block of code that might throw an error.",
    "shel": "A shell command.",
    "avail": "Checks if a variable is available.",
    "nil": "Represents a null or empty value.",
    "own": "Represents ownership of a variable.",
    "funcall": "Calls a function.",
    "check": "Starts a conditional block (if).",
    "alter": "An alternative branch in a conditional block (else if).",
    "altern": "The final branch in a conditional block (else).",
    "traverse": "A loop that iterates over a range of values.",
    "until": "A loop that continues until a condition is met.",
    "conclude": "The `finally` block in an `attempt...trap` structure."
};
function activate(context) {
    const completionProvider = vscode.languages.registerCompletionItemProvider('nervestack', {
        provideCompletionItems(document, position) {
            return allKeywords.map(keyword => new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword));
        }
    });
    const hoverProvider = vscode.languages.registerHoverProvider('nervestack', {
        provideHover(document, position) {
            const range = document.getWordRangeAtPosition(position);
            const word = document.getText(range);
            if (keywordDescriptions[word]) {
                return new vscode.Hover(new vscode.MarkdownString(keywordDescriptions[word]));
            }
            return null;
        }
    });
    context.subscriptions.push(completionProvider, hoverProvider);
}
function deactivate() { }
//# sourceMappingURL=extension.js.map