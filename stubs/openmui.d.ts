// openmui — the script binding API (component-binding §5.2): what
// `import { … } from "openmui"` gives a script, written by hand and normative.
// The builders of the components a runtime registers are declared with it,
// one for each (core.d.ts for the Core profile, generated from the registry;
// a package's own from its contracts); a tool that type-checks an application
// puts them in this module beside what is here.
//
// Two tiers (D-158), one path: the everyday API first — state, expressions,
// regions, mounting, verbs, models, themes, messages — what most scripts
// need; then the full API — any element by its type name, the tree as the
// protocol sees it (`find`, `act`, `snapshot`), view documents — for what the
// first does not cover. The everyday calls are made of the full ones.
//
// ── the everyday API ─────────────────────────────────────────────────────

/** A value as JSON carries it: what signals, models, verbs and natives exchange. */
export type Json = null | boolean | number | string | Json[] | { [key: string]: Json };

/** A value, or an expression of one, or a signal holding one: what a property and an operator take.
 *  Text may also be a message (`tr("inbox.count", { count })`). */
export type Operand<T = unknown> = T | Expr<T> | Signal<T> | (unknown extends T ? Tr : [T] extends [string] ? Tr : never);

/** An expression the runtime evaluates (CEL-compatible text, registry/expr.toml), with the signals it reads.
 *  `T` documents what it gives; the language is typed when the runtime reads it, so any expression fits any property. */
export declare class Expr<T = unknown> {
    readonly text: string;
    readonly signals: readonly number[];
    /** A field of a map or a row: `row.field("title")` (a row also reads `row.title`). */
    field(name: string): Expr;
    /** An element of a list, or a value of a map: `list.at(0)`, `map.at("key")`. */
    at(index: Operand<number | string>): Expr;
    add(other: Operand): Expr;
    sub(other: Operand<number>): Expr<number>;
    mul(other: Operand<number>): Expr<number>;
    div(other: Operand<number>): Expr<number>;
    mod(other: Operand<number>): Expr<number>;
    eq(other: Operand): Expr<boolean>;
    ne(other: Operand): Expr<boolean>;
    lt(other: Operand): Expr<boolean>;
    le(other: Operand): Expr<boolean>;
    gt(other: Operand): Expr<boolean>;
    ge(other: Operand): Expr<boolean>;
    and(other: Operand<boolean>): Expr<boolean>;
    or(other: Operand<boolean>): Expr<boolean>;
    not(): Expr<boolean>;
    neg(): Expr<number>;
}

/** State the runtime holds; used in a property it is live, and a bindable property writes it back. */
export declare class Signal<T = unknown> {
    readonly handle: number;
    readonly expr: Expr<T>;
    get(): T;
    set(value: T): void;
    value: T;
    update(change: (value: T) => T): void;
    watch(listener: (value: T) => void): void;
    field(name: string): Expr;
    at(index: Operand<number | string>): Expr;
    add(other: Operand): Expr;
    sub(other: Operand<number>): Expr<number>;
    mul(other: Operand<number>): Expr<number>;
    div(other: Operand<number>): Expr<number>;
    mod(other: Operand<number>): Expr<number>;
    eq(other: Operand): Expr<boolean>;
    ne(other: Operand): Expr<boolean>;
    lt(other: Operand): Expr<boolean>;
    le(other: Operand): Expr<boolean>;
    gt(other: Operand): Expr<boolean>;
    ge(other: Operand): Expr<boolean>;
    and(other: Operand<boolean>): Expr<boolean>;
    or(other: Operand<boolean>): Expr<boolean>;
    not(): Expr<boolean>;
    neg(): Expr<number>;
}

/** A signal of the runtime; `key` keeps its value across a reload. */
export declare function signal<T>(initial: T, key?: string): Signal<T>;

export declare function add(a: Operand, b: Operand): Expr;
export declare function sub(a: Operand<number>, b: Operand<number>): Expr<number>;
export declare function mul(a: Operand<number>, b: Operand<number>): Expr<number>;
export declare function div(a: Operand<number>, b: Operand<number>): Expr<number>;
export declare function mod(a: Operand<number>, b: Operand<number>): Expr<number>;
export declare function neg(a: Operand<number>): Expr<number>;
export declare function eq(a: Operand, b: Operand): Expr<boolean>;
export declare function ne(a: Operand, b: Operand): Expr<boolean>;
export declare function lt(a: Operand, b: Operand): Expr<boolean>;
export declare function le(a: Operand, b: Operand): Expr<boolean>;
export declare function gt(a: Operand, b: Operand): Expr<boolean>;
export declare function ge(a: Operand, b: Operand): Expr<boolean>;
export declare function and(a: Operand<boolean>, b: Operand<boolean>): Expr<boolean>;
export declare function or(a: Operand<boolean>, b: Operand<boolean>): Expr<boolean>;
export declare function not(a: Operand<boolean>): Expr<boolean>;
/** A built-in function of the expression language (registry/expr.toml): `call("max", a, b)`. */
export declare function call(name: string, ...args: Operand[]): Expr;
/** `fmt("{} to go", remaining)`: text with the arguments in its braces. */
export declare function fmt(pattern: string, ...args: Operand[]): Expr<string>;
/** `select(cond, then, otherwise)`: one of two values as the condition says. */
export declare function select<T>(cond: Operand<boolean>, then: Operand<T>, otherwise: Operand<T>): Expr<T>;

/** A message by its key, its arguments by name (MessageFormat 2): `tr("inbox.count", { count })`. */
export declare class Tr {
    readonly key: string;
}
export declare function tr(key: string, args?: { [name: string]: Operand }): Tr;

/** A view: what a builder, `when`, `each` and `slot` make; `mount` shows one. */
export declare class Element {
    protected readonly __element: true;
}
export type View = Element;
/** What an element contains: elements, lists of them; `null`, `undefined` and `false` are left out. */
export type Children = View | null | undefined | false | readonly Children[];

/** A handler of an event: the event's arguments, and in a list the rows it sits in (`event.rows.row.key`). */
export type Handler<Args = {}> = (event: Args & { readonly rows: any }) => void;

/** How a property changes over time: a style's motion rule by name, or the transition itself. */
export type Animate = { [property: string]: string | { [field: string]: Json } };

/** A row of a model as a view of it reads it: each field an expression (`row.title`, `row.author.name`). */
export type Row = Expr & { readonly [field: string]: Row };

/** What every node takes besides its own properties (spec S-8, the view schema's runtime properties).
 *  A length is in logical pixels or a theme's token (`gap: "m"`, `radius: "small"`, `padding: "xl"`). */
export interface NodeProps {
    /** An explicit id: give one to anything a person or an agent operates; never display text. */
    id?: string;
    animate?: Animate;
    align?: Operand<string>;
    alignContent?: Operand<string>;
    alignItems?: Operand<string>;
    alignSelf?: Operand<string>;
    answer?: Operand;
    aspectRatio?: Operand<number>;
    autoColumns?: Operand;
    autoRows?: Operand;
    background?: Operand<string>;
    basis?: Operand<number | string>;
    borderColor?: Operand<string>;
    borderWidth?: Operand<number | string>;
    bottom?: Operand<number | string>;
    clip?: Operand<boolean>;
    column?: Operand;
    columnGap?: Operand<number | string>;
    columns?: Operand;
    description?: Operand<string>;
    direction?: Operand<string>;
    disabled?: Operand<boolean>;
    display?: Operand<string>;
    effect?: Operand;
    focusable?: Operand<boolean>;
    gap?: Operand<number | string>;
    glyphMotion?: Operand;
    gridFlow?: Operand<string>;
    grow?: Operand<number>;
    height?: Operand<number | string>;
    justify?: Operand<string>;
    justifySelf?: Operand<string>;
    label?: Operand<string>;
    left?: Operand<number | string>;
    margin?: Operand;
    maxHeight?: Operand<number | string>;
    maxWidth?: Operand<number | string>;
    minHeight?: Operand<number | string>;
    minWidth?: Operand<number | string>;
    opacity?: Operand<number>;
    origin?: Operand;
    overflow?: Operand<string>;
    padding?: Operand;
    paddingBottom?: Operand<number | string>;
    paddingLeft?: Operand<number | string>;
    paddingRight?: Operand<number | string>;
    paddingTop?: Operand<number | string>;
    paddingX?: Operand<number | string>;
    paddingY?: Operand<number | string>;
    person?: Operand<boolean>;
    pixelEffect?: Operand;
    pointerEvents?: Operand<string>;
    position?: Operand<string>;
    radii?: Operand;
    radius?: Operand<number | string>;
    readonly?: Operand<boolean>;
    right?: Operand<number | string>;
    role?: Operand<string>;
    rotation?: Operand<number>;
    row?: Operand;
    rowGap?: Operand<number | string>;
    rows?: Operand;
    scale?: Operand<number>;
    selected?: Operand<boolean>;
    shadow?: Operand;
    shrink?: Operand<number>;
    themeScope?: Operand<string>;
    top?: Operand<number | string>;
    translateX?: Operand<number>;
    translateY?: Operand<number>;
    undoScope?: Operand<boolean>;
    verb?: Operand<string>;
    verbArgs?: Operand;
    visible?: Operand<boolean>;
    width?: Operand<number | string>;
    windowArea?: Operand<string>;
    wrap?: Operand<string | boolean>;
    onActivate?: Handler<{ [field: string]: Json }>;
    onBlur?: Handler<{ [field: string]: Json }>;
    onCloseRequest?: Handler<{ [field: string]: Json }>;
    onFocus?: Handler<{ [field: string]: Json }>;
    onGamepadAxis?: Handler<{ [field: string]: Json }>;
    onGamepadConnect?: Handler<{ [field: string]: Json }>;
    onGamepadDisconnect?: Handler<{ [field: string]: Json }>;
    onGamepadDown?: Handler<{ [field: string]: Json }>;
    onGamepadUp?: Handler<{ [field: string]: Json }>;
    onKeyDown?: Handler<{ [field: string]: Json }>;
    onKeyUp?: Handler<{ [field: string]: Json }>;
    onPointerCancel?: Handler<{ [field: string]: Json }>;
    onPointerDown?: Handler<{ [field: string]: Json }>;
    onPointerEnter?: Handler<{ [field: string]: Json }>;
    onPointerLeave?: Handler<{ [field: string]: Json }>;
    onPointerMove?: Handler<{ [field: string]: Json }>;
    onPointerUp?: Handler<{ [field: string]: Json }>;
    onPreedit?: Handler<{ [field: string]: Json }>;
    onPress?: Handler<{ [field: string]: Json }>;
    onTextInput?: Handler<{ [field: string]: Json }>;
    onWheel?: Handler<{ [field: string]: Json }>;
}

/** A region shown while the condition holds, `otherwise` when it does not. */
export declare function when(cond: Operand<boolean>, then: Children, otherwise?: Children): View;
/** A view of every row of a model, kept by key; `where` filters by the row's own fields. */
export declare function each(model: string, body: (row: Row) => View, where?: Operand<boolean> | ((row: Row) => Operand<boolean>)): View;
/** A place another view fills: a view document mounted under its name, or `mount(name, …)`. */
export declare function slot(name: string): View;

/** What a slot shows: `mount("main", column({ … }, [ … ]))`. */
export declare function mount(slot: string, view: View): void;
/** How much a verb changes (openas §2.4, registry/effects.toml): reading, a change that can be undone, one that cannot, or one outside the application. */
export type Effect = "read" | "reversible" | "irreversible" | "external";
export interface VerbOptions {
    effect?: Effect;
    summary?: string;
    /** Only a person may run it. */
    person?: boolean;
}
/** An operation of the application, by name, for a person's controls and for agents. */
export declare function verb(name: string, options: VerbOptions, run: (args: any) => Json | void): void;
export declare function verb(name: string, run: (args: any) => Json | void): void;
/** Runs a verb as the script; its answer, or an OmuiError. */
export declare function invoke(name: string, args?: { [name: string]: Json }): any;

/** A model of the runtime: rows a view shows with `each`. */
export interface Model<T = any> {
    rows(): T[];
    splice(from: number, remove: number, rows?: T[]): void;
    push(rows: T[]): void;
    clear(): void;
}
export declare function model<T = any>(name: string): Model<T>;

/** The runtime's theme: a theme, then overlays — `theme("dark", "compact")`. */
export declare function theme(...layers: string[]): void;
export declare function theme(layers: readonly string[]): void;
export declare function log(...values: unknown[]): void;
/** What a call into the runtime refused, with its code. */
export declare class OmuiError extends Error {}

// ── the full API ─────────────────────────────────────────────────────────

/** An element of any registered type by its name: what the builders call. */
export declare function element(type: string, children: Children): View;
export declare function element(type: string, props?: NodeProps & { [property: string]: unknown }, children?: Children): View;

export declare function unmount(slot: string): void;
/** The view as a view document (JSON), as the runtime reads it. */
export declare function document(view: View): Json;

/** A node found by id or selector: its properties read and written. */
export interface Found {
    readonly handle: number;
    get(property: string): any;
    set(properties: { [property: string]: Json }): void;
}
export declare function find(selector: string | { [field: string]: Json }): Found;
/** An operation on the tree as an agent asks for one (protocol `act`). */
export declare function act(request: { [field: string]: Json }): any;
/** The semantic tree, or a part of it (protocol `snapshot`). */
export declare function snapshot(query?: { [field: string]: Json }): any;

/** The themes and overlays, and the composition in use. */
export declare function themes(): any;

export declare const nil: null;

