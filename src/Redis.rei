type existence =
  | XX
  | NX;

type expiration =
  | EX(int)
  | PX(int);

module Cursor: {
  type t;

  let start: t;

  let isLast: t => bool;
};

module Error: {
  type redisError = {
    message: option(string),
    stack: option(Js.Json.t),
  };
  type replyError = {
    message: option(string),
    stack: option(Js.Json.t),
    args: option(array(Js.Json.t)),
    command: option(string),
    code: option(string),
  };
  type parserError = {
    message: option(string),
    stack: option(Js.Json.t),
    buffer: option(string),
    offset: option(int),
  };
  type abortError = {
    message: option(string),
    stack: option(Js.Json.t),
    args: option(array(Js.Json.t)),
    command: option(string),
  };
  type interruptError = {
    message: option(string),
    stack: option(Js.Json.t),
    args: option(array(Js.Json.t)),
    command: option(string),
    origin: option(Js.Exn.t),
  };
  type unknownError = {
    name: option(string),
    message: option(string),
    stack: option(Js.Json.t),
  };
  type t =
    | RedisError(redisError)
    | ReplyError(replyError)
    | ParserError(parserError)
    | AbortError(abortError)
    | InterruptError(interruptError)
    | UnknownError(unknownError);
};

module Promise: {
  type t('a);
  let make: unit => (t('a), 'a => unit);
  let resolved: 'a => t('a);
  let andThen: ('a => t('b), t('a)) => t('b);
  let map: ('a => 'b, t('a)) => t('b);
  let wait: ('a => unit, t('a)) => unit;
  let all: list(t('a)) => t(list('a));
  let race: list(t('a)) => t('a);
};

module SimpleStringReply: {
  type t =
    | Ok
    | Empty
    | Unknown(string);
};

module BulkStringReply: {type t = option(string);};

module IntegerReply: {type t = int;};

module BooleanReply: {type t = bool;};

module ScanReply: {type t('a) = (Cursor.t, 'a);};

type t;

let make: unit => t;

let quit: t => Promise.t(Belt.Result.t(SimpleStringReply.t, Error.t));

let set:
  (
    ~key: string,
    ~value: string,
    ~expiration: expiration=?,
    ~existence: existence=?,
    t
  ) =>
  Promise.t(Belt.Result.t(SimpleStringReply.t, Error.t));

let get:
  (~key: string, t) => Promise.t(Belt.Result.t(BulkStringReply.t, Error.t));

let del:
  (~keys: list(string), t) =>
  Promise.t(Belt.Result.t(IntegerReply.t, Error.t));

let exists:
  (~key: string, t) => Promise.t(Belt.Result.t(BooleanReply.t, Error.t));

let scan:
  (~cursor: Cursor.t, ~match: string=?, ~count: int=?, t) =>
  Promise.t(Belt.Result.t(ScanReply.t(list(string)), Error.t));

let hincrby:
  (~key: string, ~field: string, ~value: int, t) =>
  Promise.t(Belt.Result.t(IntegerReply.t, Error.t));

let hmset:
  (~key: string, ~values: Js.Dict.t(string), t) =>
  Promise.t(Belt.Result.t(SimpleStringReply.t, Error.t));

let hscan:
  (~key: string, ~cursor: Cursor.t, ~match: string=?, ~count: int=?, t) =>
  Promise.t(Belt.Result.t(ScanReply.t(Js.Dict.t(string)), Error.t));

let sadd:
  (~key: string, ~members: list(string), t) =>
  Promise.t(Belt.Result.t(IntegerReply.t, Error.t));

let scard:
  (~key: string, t) => Promise.t(Belt.Result.t(IntegerReply.t, Error.t));

let sismember:
  (~key: string, ~member: string, t) =>
  Promise.t(Belt.Result.t(BooleanReply.t, Error.t));
