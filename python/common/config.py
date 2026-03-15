"""Shared YAML configuration loader with environment variable overrides.

Usage::

    from common.config import load_config

    cfg = load_config("hello_world")
    host = cfg["network"]["server"]["host"]
    port = cfg["network"]["server"]["port"]

Override any value with ``OPENSOMEIP_<SECTION>_<KEY>``::

    OPENSOMEIP_SERVER_PORT=31000 python hello_world/server.py
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path
from typing import Any

import yaml

_REPO_ROOT = Path(__file__).resolve().parent.parent.parent
_CONFIG_DIR = _REPO_ROOT / "config"

_ENV_PREFIX = "OPENSOMEIP_"


def _apply_env_overrides(cfg: dict[str, Any]) -> dict[str, Any]:
    """Walk the config tree and override leaves whose flattened key matches
    an environment variable named ``OPENSOMEIP_<SECTION>_<KEY>`` (upper-case).

    For example, the YAML path ``network.server.port`` is overridden by
    ``OPENSOMEIP_SERVER_PORT``.  Only the last two path components are used
    so that the prefix stays short.
    """
    def _walk(node: dict[str, Any], parents: tuple[str, ...] = ()) -> None:
        for key, value in node.items():
            path = parents + (key,)
            if isinstance(value, dict):
                _walk(value, path)
            else:
                env_key = _ENV_PREFIX + "_".join(path[-2:]).upper()
                env_val = os.environ.get(env_key)
                if env_val is not None:
                    node[key] = _coerce(env_val, value)

    _walk(cfg)
    return cfg


def _coerce(env_val: str, original: Any) -> Any:
    """Convert a string from the environment to the type of *original*."""
    if isinstance(original, bool):
        return env_val.lower() in ("1", "true", "yes", "on")
    if isinstance(original, int):
        return int(env_val, 0)
    if isinstance(original, float):
        return float(env_val)
    return env_val


def _yaml_int_constructor(loader: yaml.SafeLoader, node: yaml.ScalarNode) -> int:
    """Parse hex literals (``0x1000``) that YAML 1.1 would treat as strings."""
    value = loader.construct_scalar(node)
    return int(value, 0)


_loader = type("_Loader", (yaml.SafeLoader,), {})
_loader.add_constructor("tag:yaml.org,2002:int", _yaml_int_constructor)
_HEX_PATTERN = "0x[0-9a-fA-F]+"
import re
_loader.add_implicit_resolver("tag:yaml.org,2002:int", re.compile(_HEX_PATTERN), list("0"))


def load_config(example_name: str, config_path: str | None = None) -> dict[str, Any]:
    """Load the YAML config for *example_name*, apply env-var overrides, and
    return the resulting dict.

    Parameters
    ----------
    example_name:
        Base name of the config file (without ``.yaml``).
    config_path:
        Optional path override (e.g. from ``--config`` CLI flag).
    """
    if config_path is not None:
        path = Path(config_path)
    else:
        path = _CONFIG_DIR / f"{example_name}.yaml"

    with open(path) as f:
        cfg: dict[str, Any] = yaml.load(f, Loader=_loader)

    return _apply_env_overrides(cfg)


def add_config_arg(parser: argparse.ArgumentParser | None = None) -> argparse.ArgumentParser:
    """Add a ``--config`` argument to *parser* (or create one)."""
    if parser is None:
        parser = argparse.ArgumentParser()
    parser.add_argument(
        "--config",
        default=None,
        help="Path to a custom YAML config file (overrides the default in config/).",
    )
    return parser
