type cursor;

let cursor: int => cursor;

type existence =
  | XX
  | NX;

type expiration =
  | EX(int)
  | PX(int);

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
  (~key: string, t) => Promise.t(Belt.Result.t(option(string), Error.t));
