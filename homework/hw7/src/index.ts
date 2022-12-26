import { Chart, ChartConfiguration, Point, registerables } from 'chart.js';

Chart.register(...registerables);

const COLORS = [
  'Red',
  'Blue',
  'Green',
  'Purple',
  'DarkGreen',
  'Orange',
  'Coral',
  'Brown',
];

const less = (best: any, curr: any) => best > curr;
const greater = (best: any, curr: any) => best < curr;

interface ChartType {
  evalFunc: string;
  compFunc: CompFunc;
  initFunc?: string;
  name: string;
}

const chartTypes: ChartType[] = [
  {
    evalFunc: 'totalIntraClusterScatter',
    compFunc: less,
    name: 'intra',
  },
  {
    evalFunc: 'totalInterClusterScatter',
    compFunc: greater,
    name: 'inter',
  },
  {
    evalFunc: 'totalCombined',
    compFunc: greater,
    name: 'combined',
  },
  {
    evalFunc: 'totalIntraClusterScatter',
    compFunc: less,
    initFunc: 'initCentroidsPP',
    name: 'intra-kmeansPP',
  },
  {
    evalFunc: 'totalInterClusterScatter',
    compFunc: greater,
    initFunc: 'initCentroidsPP',
    name: 'inter-kmeansPP',
  },
  {
    evalFunc: 'totalCombined',
    compFunc: greater,
    initFunc: 'initCentroidsPP',
    name: 'combined-kmeansPP',
  },
];

type CompFunc = (best: any, curr: any) => any;

let charts: Chart[] = Array(chartTypes.length);
let intervals: NodeJS.Timer[] = [];

interface ChartConfig {
  title: string;
}

const chartConfig = ({ title }: ChartConfig): ChartConfiguration => ({
  type: 'scatter',
  data: {
    datasets: [],
  },
  options: {
    animation: false,
    elements: {
      point: {
        radius: 5,
        borderWidth: 0,
      },
    },
    scales: {
      y: {
        display: false,
      },
      x: {
        display: false,
      },
    },
    plugins: {
      title: {
        display: true,
        text: title,
        font: {
          size: 40,
        },
      },
    },
  },
});

type KMeansConfig = {
  name: string;
  compare: CompFunc;
  evaluation: any;
  init?: any;
};

class KMeans {
  private MAX_ITER = 25;
  private MAX_RESTARTS = 100;

  private data: Point[] = [];
  private centroids: Point[] = [];
  private clusters: Point[][] = [];
  private prevCentroids: Point[] = [];
  private chart?: Chart;

  private randomIndex(length: number) {
    return Math.floor(Math.random() * length);
  }

  private norm(point: Point) {
    return Math.sqrt(point.x ** 2 + point.y ** 2);
  }

  private mean(data: Point[]) {
    const sum = data.reduce((prev, curr) => ({
      x: prev.x + curr.x,
      y: prev.y + curr.y,
    }));
    return { x: sum.x / data.length, y: sum.y / data.length };
  }

  private distance(p1: Point, p2: Point) {
    return this.norm({ x: p1.x - p2.x, y: p1.y - p2.y });
  }

  private initCentroids(k: number) {
    const chosen = new Set();
    this.centroids = new Array(k);

    for (let i = 0; i < k; i++) {
      let index = this.randomIndex(this.data.length);

      while (chosen.has(index)) {
        index = this.randomIndex(this.data.length);
      }

      this.centroids[i] = this.data[index];
      chosen.add(index);
    }
  }

  private initCentroidsPP(k: number) {
    const chosen = new Set();
    this.centroids = new Array(k);
    this.centroids[0] = this.data[this.randomIndex(this.data.length)];

    for (let i = 1; i < k; i++) {
      const distances = this.data
        .map((point, index) => {
          if (chosen.has(index)) return { dist: 0, index };

          let min = Number.MAX_SAFE_INTEGER;
          for (let j = 0; j < i; j++) {
            min = Math.min(min, this.distance(point, this.centroids[j]) ** 2);
          }
          return { dist: min ?? 0, index };
        })
        .filter((_) => _.dist !== 0);

      const sum = distances
        .map((_) => _.dist)
        .reduce((prev, curr) => prev + curr);

      const distribution = distances
        .map(({ dist, index }) => ({
          prop: Math.ceil((dist / sum) * distances.length),
          index,
        }))
        .map(({ prop, index }) => Array(prop).fill(index))
        .flat();

      const index = distribution[this.randomIndex(distribution.length)];
      this.centroids[i] = this.data[index];
      chosen.add(index);
    }
  }

  private intraClusterScatter(index: number) {
    // average diameter distance
    //
    return (
      this.clusters[index]
        .map((p1, i) =>
          [
            ...this.clusters[index].slice(0, i),
            ...this.clusters[index].slice(i + 1),
          ]
            .map((p2) => this.distance(p1, p2))
            .reduce((prev, curr) => prev + curr, 0)
        )
        .reduce((prev, curr) => prev + curr, 0) /
      (this.clusters[index].length * (this.clusters[index].length - 1))
    );

    // complete diameter distance
    //
    // return Math.max(
    //   ...this.clusters[index]
    //     .map((p1, i) =>
    //       Math.max(
    //         ...this.clusters[index]
    //           .slice(i + 1)
    //           .map((p2) => this.distance(p1, p2)),
    //         0
    //       )
    //     )
    //     .flat(),
    //   0
    // );
  }

  private totalIntraClusterScatter() {
    return (
      this.clusters
        .map((_, index) => this.intraClusterScatter(index))
        .reduce((prev, curr) => prev + curr) / this.clusters.length
    );
  }

  private interClusterScatter(i1: number, i2: number) {
    // average linkage distance
    //
    // return (
    //   this.clusters[i1]
    //     .map((p1) =>
    //       this.clusters[i2]
    //         .map((p2) => this.distance(p1, p2))
    //         .reduce((prev, curr) => prev + curr)
    //     )
    //     .reduce((prev, curr) => prev + curr) /
    //   (this.clusters[i1].length * this.clusters[i2].length)
    // );

    // single linkage distance
    //
    // return Math.min(
    //   ...this.clusters[i1].map((p1) =>
    //     Math.min(...this.clusters[i2].map((p2) => this.distance(p1, p2)))
    //   )
    // );

    // average centroid linkage distance
    //
    return (
      (this.clusters[i1]
        .map((p1) => this.distance(p1, this.centroids[i1]))
        .reduce((prev, curr) => prev + curr) +
        this.clusters[i2]
          .map((p1) => this.distance(p1, this.centroids[i1]))
          .reduce((prev, curr) => prev + curr)) /
      (this.clusters[i1].length + this.clusters[i2].length)
    );
  }

  private totalInterClusterScatter() {
    return (
      this.clusters
        .map((_, i1) => {
          return this.clusters
            .slice(i1 + 1)
            .map((_, i2) => this.interClusterScatter(i1, i2))
            .reduce((prev, curr) => prev + curr, 0);
        })
        .reduce((prev, curr) => prev + curr) / this.centroids.length
    );
  }

  private totalCombined() {
    // Dunn Index
    //
    const minInter = Math.min(
      ...this.clusters
        .map((_, i1) => {
          let scatters = [];
          for (let i2 = i1 + 1; i2 < this.clusters.length; i2++) {
            scatters.push(this.interClusterScatter(i1, i2));
          }
          return scatters;
        })
        .flat()
    );

    const maxIntra = Math.max(
      ...this.clusters.map((_, i) => this.intraClusterScatter(i))
    );

    return minInter / maxIntra;
  }

  private updateClusters() {
    this.clusters = new Array(this.centroids.length).fill(null).map((_) => []);

    this.data.forEach((point) => {
      const distances = this.centroids
        .map((centroid, index) => ({
          index,
          dist: this.distance(centroid, point),
        }))
        .sort((a, b) => a.dist - b.dist);

      this.clusters[distances[0].index].push(point);
    });

    this.clusters.forEach((cluster, index) =>
      cluster.push(this.centroids[index])
    );
  }

  private updateCentroids() {
    this.centroids = this.centroids.map((_, index) =>
      this.mean(this.clusters[index])
    );
  }

  private updateChart() {
    const updateContext = (context: any, last: any, rest: any) => {
      const dataIndex = context.dataIndex;
      const data = context.dataset.data;

      return dataIndex === data.length - 1 ? last : rest;
    };

    if (this.chart) {
      this.chart.data.datasets = this.clusters.map((cluster, index) => ({
        label: `Cluster ${index + 1}`,
        data: cluster,
        pointBackgroundColor: (context: any) =>
          updateContext(context, 'Black', COLORS[index]),
        pointBorderColor: (context: any) =>
          updateContext(context, 'White', COLORS[index]),
        pointBorderWidth: (context: any) => updateContext(context, 2, 0),
      }));
      this.chart.update();
    }
  }

  private reachedLocalExtrema() {
    return this.centroids.every(
      (centroid, index) =>
        this.prevCentroids[index].x === centroid.x &&
        this.prevCentroids[index].y === centroid.y
    );
  }

  private restart(clusters: number, init: any) {
    init(clusters);
    this.updateClusters();
  }

  constructor(data?: string, chart?: Chart) {
    if (data) {
      try {
        this.chart = chart;

        this.data = data
          .split('\n')
          .map((line) =>
            line
              .trim()
              .split(/\s+/)
              .map((_) => Number(_))
          )
          .map(([x, y]) => ({ x, y }));
      } catch (e) {
        console.error(e);
      }
    }
  }

  clusterize(
    clusters: number,
    {
      name,
      compare,
      evaluation,
      init = this.initCentroids.bind(this),
    }: KMeansConfig
  ) {
    let restarts = this.MAX_RESTARTS;
    let bestSoFar: number | null = null;

    intervals.push(
      setInterval(() => {
        if (restarts-- > 0) {
          this.restart(clusters, init.bind(this));

          let iter = this.MAX_ITER;

          while (iter-- > 0) {
            this.prevCentroids = this.centroids;

            this.updateCentroids();
            this.updateClusters();

            if (this.reachedLocalExtrema()) {
              break;
            }
          }

          const current = evaluation();

          if (!bestSoFar || compare(bestSoFar, current)) {
            bestSoFar = current;
            this.updateChart();
          }

          console.log(`${name} => Current: ${current} | Best: ${bestSoFar}`);
        }
      })
    );
  }
}

document.getElementById('file')?.addEventListener('change', (event: any) => {
  document.getElementById('fileName')!.innerHTML =
    event.target.files[0]?.name ?? 'No dataset';
});

document.getElementById('data')?.addEventListener('submit', (event: any) => {
  event.preventDefault();

  const wrapper = document.getElementsByClassName('charts-wrapper')[0];
  wrapper.innerHTML = '';

  const file: File | undefined = (
    document.getElementById('file') as HTMLInputElement
  ).files?.[0];

  if (!file) {
    throw 'missing dataset';
  }

  const clusters = Number(
    (document.getElementById('clusters') as HTMLInputElement).value
  );

  intervals.forEach((interval) => clearInterval(interval));
  intervals = [];

  let reader = new FileReader();
  reader.onload = () => {
    chartTypes.forEach((type, index) => {
      if (charts[index]) {
        charts[index].destroy();
      }

      const chartElement = document.createElement('div');
      chartElement.classList.add('chart');
      chartElement.innerHTML = `<canvas id="chart-${type.name}"></canvas>`;
      wrapper.appendChild(chartElement);

      charts[index] = new Chart(
        chartElement.querySelector(`#chart-${type.name}`) as HTMLCanvasElement,
        chartConfig({ title: `${file.name.split('.')[0]}-${type.name}` })
      );

      const kMeans = new KMeans(reader.result as string, charts[index]);
      kMeans.clusterize(clusters, {
        name: type.name,
        compare: type.compFunc,
        evaluation: kMeans[type.evalFunc as keyof KMeans]?.bind(kMeans),
        init: kMeans[type.initFunc as keyof KMeans]?.bind(kMeans),
      });
    });
  };
  reader.readAsText(file, 'utf8');
});
