from jinja2 import Environment, FileSystemLoader
from pathlib import Path
import os


def generate_metadata():
    meta_dir = Path(__file__).parent
    p = meta_dir / "templates"
    env = Environment(loader=FileSystemLoader(str(p)))
    template = env.get_template("meta.yaml.jinja")
    rendered = template.render(
        git_hash=os.environ["GIT_HASH"],
        gdx_version=os.environ["GDX_VERSION"],
    )

    with open(meta_dir / "meta.yaml", "w") as outfile:
        outfile.write(rendered)


def main():
    generate_metadata()


if __name__ == "__main__":
    main()
